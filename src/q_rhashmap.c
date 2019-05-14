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

#include "q_rhashmap.h"
#include "fastdiv.h"
#include "utils.h"

#define	HASH_INIT_SIZE		(16)
#define	MAX_GROWTH_STEP		(1024U * 1024)

#define	APPROX_85_PERCENT(x)	(((x) * 870) >> 10)
#define	APPROX_40_PERCENT(x)	(((x) * 409) >> 10)


static int __attribute__((__unused__))
validate_psl_p(
    q_rhashmap_t *hmap, 
    const q_rh_bucket_t *bucket, 
    uint32_t i
    )
{
  uint32_t base_i = fast_rem32(bucket->hash, hmap->size, hmap->divinfo);
  uint32_t diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
  return bucket->key == 0 || diff == bucket->psl;
}

/*
 * rhashmap_get: lookup an value given the key.
 *
 * => If key is present, return its associated value; otherwise NULL.
 */
int 
q_rhashmap_get(
    q_rhashmap_t *hmap, 
    KEYTYPE  key,
    VALTYPE *ptr_val,
    bool *ptr_is_found
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(KEYTYPE), hmap->hashkey);
  uint32_t n = 0; 
  uint32_t i = fast_rem32(hash, hmap->size, hmap->divinfo);
  q_rh_bucket_t *bucket = NULL;
  *ptr_is_found = false;
  *ptr_val      = 0;

  /*
   * Lookup is a linear probe.
   */
probe:
  bucket = &hmap->buckets[i];
  ASSERT(validate_psl_p(hmap, bucket, i));

  if ( ( bucket->hash == hash ) && ( bucket->key == key ) ) {
    *ptr_val = bucket->val;
    *ptr_is_found = true;
    goto BYE;
  }

  /*
   * Stop probing if we hit an empty bucket; also, if we hit a
   * bucket with PSL lower than the distance from the base location,
   * then it means that we found the "rich" bucket which should
   * have been captured, if the key was inserted -- see the central
   * point of the algorithm in the insertion function.
   */
  if (!bucket->key || n > bucket->psl) {
    *ptr_is_found = false;
    goto BYE;
  }
  n++;

  /* Continue to the next bucket. */
  i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
  goto probe;
BYE:
  return status;
}

/*
 * rhashmap_insert: internal rhashmap_put(), without the resize.
 */
static int
q_rhashmap_insert(
    q_rhashmap_t *hmap, 
    KEYTYPE key,
    VALTYPE val,
    int update_type,
    VALTYPE *ptr_oldval
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(KEYTYPE), hmap->hashkey);
  q_rh_bucket_t *bucket, entry;
  uint32_t i;

  // 0 is not a valid value for a key
  if ( key == 0 ) { go_BYE(-1); }

  // Setup the bucket entry.
  entry.key   = key;
  entry.hash  = hash;
  entry.val   = val;
  entry.psl   = 0;
  *ptr_oldval = 0;

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
      *ptr_oldval = bucket->val;
      if ( update_type == Q_RHM_SET ) { 
        bucket->val = val;
      }
      else if ( update_type == Q_RHM_INCR ) { 
        bucket->val += val;
      }
      else {
        go_BYE(-1);
      }
      goto BYE;
    }

    /*
     * We found a "rich" bucket.  Capture its location.
     */
    if (entry.psl > bucket->psl) {
      q_rh_bucket_t tmp;

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
BYE:
  return status;
}

static int
q_rhashmap_resize(
    q_rhashmap_t *hmap, 
    size_t newsize
    )
{
  int status = 0;
  q_rh_bucket_t *oldbuckets = hmap->buckets;
  const size_t oldsize = hmap->size;
  q_rh_bucket_t *newbuckets = NULL;
  const size_t len = newsize * sizeof(q_rh_bucket_t);

  if ( ( oldbuckets == NULL ) && ( oldsize != 0 ) ) { go_BYE(-1); }
  if ( ( oldbuckets != NULL ) && ( oldsize == 0 ) ) { go_BYE(-1); }
  if ( ( newsize <= 0 ) || ( newsize >= UINT_MAX ) )  { go_BYE(-1); }
  if ( newsize < hmap->nitems ) { go_BYE(-1); }

  // Check for an overflow and allocate buckets.  
  newbuckets = malloc(len);
  return_if_malloc_failed(newbuckets);
  memset(newbuckets, '\0', len);

  hmap->buckets = newbuckets;
  hmap->size    = newsize;
  hmap->nitems  = 0;

   // generate a new hash key/seed every time we resize the hash table.
  hmap->divinfo = fast_div32_init(newsize);
  hmap->hashkey ^= random() | (random() << 32);

  for ( uint32_t i = 0; i < oldsize; i++) {
    const q_rh_bucket_t *bucket = &oldbuckets[i];

    /* Skip the empty buckets. */
    if ( !bucket->key ) { continue; }

    VALTYPE oldval; // not needed except for signature
    q_rhashmap_insert(hmap, bucket->key, bucket->val, Q_RHM_SET, &oldval);
  }
  free_if_non_null(oldbuckets);
BYE:
  return status;
}

/*
 * rhashmap_put: insert a value given the key.
 *
 * => If the key is already present, return its associated value.
 * => Otherwise, on successful insert, return the given value.
 */
int
q_rhashmap_put(
    q_rhashmap_t *hmap, 
    KEYTYPE key, 
    VALTYPE val,
    int update_type,
    VALTYPE *ptr_oldval
    )
{
  int status = 0;
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
    status = q_rhashmap_resize(hmap, newsize); cBYE(status);
  }
  status = q_rhashmap_insert(hmap, key, val, update_type, ptr_oldval);
  cBYE(status);
BYE:
  return status;
}

/*
 * rhashmap_del: remove the given key and return its value.
 *
 * => If key was present, return its associated value; otherwise NULL.
 */
int
q_rhashmap_del(
    q_rhashmap_t *hmap, 
    KEYTYPE key,
    bool *ptr_key_exists,
    VALTYPE *ptr_oldval
    )
{
  int status = 0;
  const size_t threshold = APPROX_40_PERCENT(hmap->size);
  const uint32_t hash = murmurhash3(&key, sizeof(KEYTYPE), hmap->hashkey);
  uint32_t n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
  q_rh_bucket_t *bucket;
  *ptr_oldval = 0;

probe:
  /*
   * The same probing logic as in the lookup function.
   */
  bucket = &hmap->buckets[i];
  if (bucket->key == 0 ) { 
    *ptr_key_exists = false; goto BYE;
  }
  if ( n > bucket->psl ) { 
    *ptr_key_exists = false; goto BYE;
  }
  ASSERT(validate_psl_p(hmap, bucket, i));

  if ( ( bucket->hash != hash ) || ( bucket->key != key ) ) { 
    /* Continue to the next bucket. */
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    n++;
    goto probe;
  }

  // Free the bucket.
  bucket->key  = 0; 
  *ptr_oldval  = bucket->val;
  *ptr_key_exists = true;
  bucket->val  = 0; 
  // TODO Should we do this? bucket->hash = 0; 
  // TODO Should we do this? bucket->psl  = 0; 
  hmap->nitems--;

  /*
   * The probe sequence must be preserved in the deletion case.
   * Use the backwards-shifting method to maintain low variance.
   */
  for ( ; ; ) {
    q_rh_bucket_t *nbucket = NULL;

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
     status = q_rhashmap_resize(hmap, newsize); cBYE(status);
  }
BYE:
  return status;
}

/*
 * rhashmap_create: construct a new hash table.
 *
 * => If size is non-zero, then pre-allocate the given number of buckets;
 * => If size is zero, then a default minimum is used.
 */
q_rhashmap_t *
q_rhashmap_create(
      size_t size
        )
{
  q_rhashmap_t *hmap = NULL;

  hmap = calloc(1, sizeof(q_rhashmap_t));
  if (!hmap) {
    return NULL;
  }
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
    q_rhashmap_t *ptr_hmap
    )
{
  free(ptr_hmap->buckets);
  free(ptr_hmap);
  ptr_hmap = NULL;
}
