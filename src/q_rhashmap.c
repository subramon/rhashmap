/*
 * Copyright (c) 2017 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

/*
 * A general purpose hash table, using the Robin Hood hashing algorithm.
 *
 * Conceptually, it is a hash table using linear probing on lookup with
 * a particular displacement strategy on inserts.  The central idea of
 * the Robin Hood hashing algorithm is to reduce the variance of the
 * probe sequence length (PSL).
 *
 * Reference:
 *
 *	Pedro Celis, 1986, Robin Hood Hashing, University of Waterloo
 *	https://cs.uwaterloo.ca/research/tr/1986/CS-86-14.pdf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <assert.h>

#include "q_rhashmap.h"
#include "fastdiv.h"
#include "utils.h"

#define	HASH_INIT_SIZE		(16)
#define	MAX_GROWTH_STEP		(1024U * 1024)

#define	APPROX_85_PERCENT(x)	(((x) * 870) >> 10)
#define	APPROX_40_PERCENT(x)	(((x) * 409) >> 10)

static inline uint32_t __attribute__((always_inline))
compute_hash(
    const q_rhashmap_t *hmap, 
    const void *key, 
    const size_t len
    )
{
  /*
   * Avoiding the use function pointers here; test and call relying
   * on branch predictors provides a better performance.
   */
  if (hmap->flags & RHM_NONCRYPTO) {
    return murmurhash3(key, len, hmap->hashkey);
  }
  return halfsiphash(key, len, hmap->hashkey);
}

static int __attribute__((__unused__))
validate_psl_p(
    q_rhashmap_t *hmap, 
    const rh_bucket_t *bucket, 
    unsigned i
    )
{
  unsigned base_i = fast_rem32(bucket->hash, hmap->size, hmap->divinfo);
  unsigned diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
  return bucket->key == 0 || diff == bucket->psl;
}

/*
 * rhashmap_get: lookup an value given the key.
 *
 * => If key is present, return its associated value; otherwise NULL.
 */
__VALTYPE__
q_rhashmap_get(
    q_rhashmap_t *hmap, 
    __KEYTYPE__  key
    )
{
  const uint32_t hash = compute_hash(hmap, &key, sizeof(__KEYTYPE__));
  unsigned n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
  rh_bucket_t *bucket;

  /*
   * Lookup is a linear probe.
   */
probe:
  bucket = &hmap->buckets[i];
  ASSERT(validate_psl_p(hmap, bucket, i));

  if ( ( bucket->hash == hash ) && ( bucket->key == key ) ) {
    return bucket->val;
  }

  /*
   * Stop probing if we hit an empty bucket; also, if we hit a
   * bucket with PSL lower than the distance from the base location,
   * then it means that we found the "rich" bucket which should
   * have been captured, if the key was inserted -- see the central
   * point of the algorithm in the insertion function.
   */
  if (!bucket->key || n > bucket->psl) {
    return 0;
  }
  n++;

  /* Continue to the next bucket. */
  i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
  goto probe;
}

/*
 * rhashmap_insert: internal rhashmap_put(), without the resize.
 */
static __VALTYPE__ // TODO CHeck this type
q_rhashmap_insert(
    q_rhashmap_t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ val
    )
{
  const uint32_t hash = compute_hash(hmap, &key, sizeof(__KEYTYPE__));
  rh_bucket_t *bucket, entry;
  unsigned i;

  ASSERT(key != 0);

  /*
   * Setup the bucket entry.
   */
  entry.key  = key;
  entry.hash = hash;
  entry.val  = val;
  entry.psl  = 0;

  /*
   * From the paper: "when inserting, if a record probes a location
   * that is already occupied, the record that has traveled longer
   * in its probe sequence keeps the location, and the other one
   * continues on its probe sequence" (page 12).
   *
   * Basically: if the probe sequence length (PSL) of the element
   * being inserted is greater than PSL of the element in the bucket,
   * then swap them and continue.
   */
  i = fast_rem32(hash, hmap->size, hmap->divinfo);
probe:
  bucket = &hmap->buckets[i];
  if (bucket->key) {
    ASSERT(validate_psl_p(hmap, bucket, i));

    /*
     * There is a key in the bucket.
     */
    if ( (bucket->hash == hash) && (bucket->key == key) ) { 
      /* Duplicate key: return the current value. 
         TODO What to do about this code?
      if ((hmap->flags & RHM_NOCOPY) == 0) {
        free(entry.key);
      }
       * */
      return bucket->val;
    }

    /*
     * We found a "rich" bucket.  Capture its location.
     */
    if (entry.psl > bucket->psl) {
      rh_bucket_t tmp;

      /*
       * Place our key-value pair by swapping the "rich"
       * bucket with our entry.  Copy the structures.
       */
      tmp = entry;
      entry = *bucket;
      *bucket = tmp;
    }
    entry.psl++;

    /* Continue to the next bucket. */
    ASSERT(validate_psl_p(hmap, bucket, i));
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    goto probe;
  }

  /*
   * Found a free bucket: insert the entry.
   */
  *bucket = entry; // copy
  hmap->nitems++;

  ASSERT(validate_psl_p(hmap, bucket, i));
  return val;
}

static int
q_rhashmap_resize(
    q_rhashmap_t *hmap, 
    size_t newsize
    )
{
  const size_t len = newsize * sizeof(rh_bucket_t);
  rh_bucket_t *oldbuckets = hmap->buckets;
  const size_t oldsize = hmap->size;
  rh_bucket_t *newbuckets;

  ASSERT(newsize > 0);
  ASSERT(newsize > hmap->nitems);

  /*
   * Check for an overflow and allocate buckets.  Also, generate
   * a new hash key/seed every time we resize the hash table.
   */
  if (newsize > UINT_MAX || (newbuckets = calloc(1, len)) == NULL) {
    return -1;
  }
  hmap->buckets = newbuckets;
  hmap->size = newsize;
  hmap->nitems = 0;

  hmap->divinfo = fast_div32_init(newsize);
  hmap->hashkey ^= random() | (random() << 32);

  for (unsigned i = 0; i < oldsize; i++) {
    const rh_bucket_t *bucket = &oldbuckets[i];

    /* Skip the empty buckets. */
    if (!bucket->key) {
      continue;
    }
    q_rhashmap_insert(hmap, bucket->key, bucket->val);
    /* What do we do about this??
    if ((hmap->flags & RHM_NOCOPY) == 0) {
      free(bucket->key);
    }
    */
  }
  if (oldbuckets) {
    free(oldbuckets);
  }
  return 0;
}

/*
 * rhashmap_put: insert a value given the key.
 *
 * => If the key is already present, return its associated value.
 * => Otherwise, on successful insert, return the given value.
 */
__VALTYPE__ 
q_rhashmap_put(
    q_rhashmap_t *hmap, 
    __KEYTYPE__ key, 
    __VALTYPE__ val
    )
{
  const size_t threshold = APPROX_85_PERCENT(hmap->size);

  /*
   * If the load factor is more than the threshold, then resize.
   */
  if (__predict_false(hmap->nitems > threshold)) {
    /*
     * Grow the hash table by doubling its size, but with
     * a limit of MAX_GROWTH_STEP.
     */
    const size_t grow_limit = hmap->size + MAX_GROWTH_STEP;
    const size_t newsize = MIN(hmap->size << 1, grow_limit);
    if (q_rhashmap_resize(hmap, newsize) != 0) {
      return 0;
    }
  }

  return q_rhashmap_insert(hmap, key, val);
}

/*
 * rhashmap_del: remove the given key and return its value.
 *
 * => If key was present, return its associated value; otherwise NULL.
 */
__VALTYPE__ 
q_rhashmap_del(
    q_rhashmap_t *hmap, 
    __KEYTYPE__ key
    )
{
  const size_t threshold = APPROX_40_PERCENT(hmap->size);
  const uint32_t hash = compute_hash(hmap, &key, sizeof(__KEYTYPE__));
  unsigned n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
  rh_bucket_t *bucket;
  __VALTYPE__ val;

probe:
  /*
   * The same probing logic as in the lookup function.
   */
  bucket = &hmap->buckets[i];
  if (!bucket->key || n > bucket->psl) {
    return 0;
  }
  ASSERT(validate_psl_p(hmap, bucket, i));

  if ( ( bucket->hash != hash ) || ( bucket->key != key ) ) { 
    /* Continue to the next bucket. */
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    n++;
    goto probe;
  }

  /*
   * Free the bucket.
   */
  if ((hmap->flags & RHM_NOCOPY) == 0) {
    bucket->key = 0; // free(bucket->key);
  }
  val = bucket->val;
  hmap->nitems--;

  /*
   * The probe sequence must be preserved in the deletion case.
   * Use the backwards-shifting method to maintain low variance.
   */
  for (;;) {
    rh_bucket_t *nbucket;

    bucket->key = 0;

    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    nbucket = &hmap->buckets[i];
    ASSERT(validate_psl_p(hmap, nbucket, i));

    /*
     * Stop if we reach an empty bucket or hit a key which
     * is in its base (original) location.
     */
    if (!nbucket->key || nbucket->psl == 0) {
      break;
    }

    nbucket->psl--;
    *bucket = *nbucket;
    bucket = nbucket;
  }

  /*
   * If the load factor is less than threshold, then shrink by
   * halving the size, but not more than the minimum size.
   */
  if (hmap->nitems > hmap->minsize && hmap->nitems < threshold) {
    size_t newsize = MAX(hmap->size >> 1, hmap->minsize);
    (void)q_rhashmap_resize(hmap, newsize);
  }
  return val;
}

/*
 * rhashmap_create: construct a new hash table.
 *
 * => If size is non-zero, then pre-allocate the given number of buckets;
 * => If size is zero, then a default minimum is used.
 */
q_rhashmap_t *
q_rhashmap_create(
      size_t size, 
        unsigned flags
        )
{
  q_rhashmap_t *hmap;

  hmap = calloc(1, sizeof(q_rhashmap_t));
  if (!hmap) {
    return NULL;
  }
  hmap->flags = flags;
  hmap->minsize = MAX(size, HASH_INIT_SIZE);
  if (q_rhashmap_resize(hmap, hmap->minsize) != 0) {
    free(hmap);
    return NULL;
  }
  ASSERT(hmap->buckets);
  ASSERT(hmap->size);
  return hmap;
}

/*
 * rhashmap_destroy: free the memory used by the hash table.
 *
 * => It is the responsibility of the caller to remove elements if needed.
 */
void
q_rhashmap_destroy(
    q_rhashmap_t *hmap
    )
{
  free(hmap->buckets);
  free(hmap);
}
