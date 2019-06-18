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

#include <omp.h>
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

//------------------------------------------------------
int 
q_rhashmap_mk_hash(
    KEYTYPE *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    )
{
  int status = 0;
  int chunk_size = 1024;
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    hashes[i] = murmurhash3(&(keys[i]), sizeof(KEYTYPE), hmap_hashkey);
  }
  return status;
}
//------------------------------------------------------
int 
q_rhashmap_mk_loc(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t hmap_size, // input 
    uint64_t hmap_divinfo, // input 
    uint32_t *locs // output [nkeys] 
    )
{
  int status = 0;
  int chunk_size = 1024;
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    locs[i] = fast_rem32(hashes[i], hmap_size, hmap_divinfo);
  }
  return status;
}
//------------------------------------------------------
int 
q_rhashmap_getn(
    q_rhashmap_t *hmap, // INPUT
    KEYTYPE *keys, // INPUT: [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] 
    VALTYPE *vals, // OUTPUT [nkeys] 
    uint32_t nkeys // INPUT 
    // TODO P4 we won't do is_found for the first implementation
    )
{
  int status = 0;
  for ( uint32_t j = 0; j < nkeys; j++ ) {
    uint32_t n = 0; 
    uint32_t i = locs[j];
    uint32_t hash = hashes[j];
    q_rh_bucket_t *bucket = NULL;
    vals[j]     = 0;

probe:
    bucket = &hmap->buckets[i];
    ASSERT(validate_psl_p(hmap, bucket, i));

    if ( ( bucket->hash == hash ) && ( bucket->key == keys[j] ) ) {
      vals[j] = bucket->val;
      continue;
    }
    if (!bucket->key || n > bucket->psl) {
      continue; // not found
    }
    n++;
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    goto probe;
  }
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
    VALTYPE *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(KEYTYPE), hmap->hashkey);
  q_rh_bucket_t *bucket, entry;
  uint32_t i;
  int num_probes = 0;

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
      else if ( update_type == Q_RHM_ADD ) { 
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
    num_probes++;
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    goto probe;
  }

  /*
   * Found a free bucket: insert the entry.
   */
  *bucket = entry; // copy
  hmap->nitems++;

  ASSERT(validate_psl_p(hmap, bucket, i));
  *ptr_num_probes = num_probes;
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
  int num_probes = 0;

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
    q_rhashmap_insert(hmap, bucket->key, bucket->val, Q_RHM_SET, &oldval,
        &num_probes);
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
    VALTYPE *ptr_oldval,
    int *ptr_num_probes
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
  status = q_rhashmap_insert(hmap, key, val, update_type, 
      ptr_oldval, ptr_num_probes);
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
    VALTYPE *ptr_oldval,
    bool *ptr_is_found
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
    *ptr_is_found = false; goto BYE;
  }
  if ( n > bucket->psl ) { 
    *ptr_is_found = false; goto BYE;
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
  *ptr_is_found = true;
  bucket->val  = 0; 
  bucket->hash = 0; 
  bucket->psl  = 0; 
  hmap->nitems--;

  /*
   * The probe sequence must be preserved in the deletion case.
   * Use the backwards-shifting method to maintain low variance.
   */
  for ( ; ; ) {
    q_rh_bucket_t *nbucket = NULL;

    bucket->key = 0;
    bucket->val  = 0; 
    bucket->hash = 0; 
    bucket->psl  = 0; 

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

// Given 
// (1) a set of keys 
// (2) hash for each key
// (3) value for each key
// Update as follows. If j^{th} key found, then 
// (A) set is_found[j]  to true
// (B) update value with new value provided (either set or add)
// Else, set is_found[j] to false
int 
q_rhashmap_putn(
    q_rhashmap_t *hmap,  // INPUT
    int update_type, // INPUT
    KEYTYPE *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys]
    VALTYPE *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *is_founds // OUTPUT [nkeys bits] TODO: Change from byte to bit 
    )
{
  int status = 0;
  int *is_new = NULL;
  int nT = 0;
  int num_new = 0;
  bool need_sequential_put = false;
  // quick sanity check 
  switch ( update_type ) { 
    case Q_RHM_SET : case Q_RHM_ADD : break;
    default: go_BYE(-1); break;
  }
  nT = omp_get_num_threads();
  is_new = malloc(nT * sizeof(int));
  return_if_malloc_failed(is_new);
  for ( int i = 0; i < nT; i++ ) { is_new[i] = 0; }
#pragma omp parallel
  {
    register uint32_t mytid = omp_get_thread_num();
    register int num_threads = omp_get_num_threads();
    register uint32_t hmap_size    = hmap->size;
    register uint64_t hmap_divinfo = hmap->divinfo;
    for ( uint32_t j = 0; j < nkeys; j++ ) {
      register uint32_t hash = hashes[j];
      register q_rh_bucket_t *buckets = hmap->buckets;
      register KEYTYPE key = keys[j];
      register VALTYPE val = vals[j];
      uint32_t i = locs[j]; // fast_rem32(hash, hmap_size, hmap_divinfo);
      // Following so that 2 threads don't deal with same key
      if ( ( hash % num_threads ) != mytid )  { continue; }
      is_founds[j] = 0;
      uint32_t n = 0; 

      for ( ; ; ) { // search until found 
        q_rh_bucket_t *bucket = buckets + i;
        ASSERT(validate_psl_p(hmap, bucket, i)); // TODO P4 needed?

        if ( ( bucket->hash == hash ) && ( bucket->key == key ) ) {
          switch ( update_type ) {
            case Q_RHM_SET : bucket->val  = val; break; 
            case Q_RHM_ADD : bucket->val += val; break;
          }
          is_founds[j] = 1;
          break; 
        }
        if ( ( !bucket->key ) || ( n > bucket->psl ) ) { // not found
          is_founds[j] = 0; 
          break; 
        }
        n++;
        i = fast_rem32(i + 1, hmap_size, hmap_divinfo);
      }
      if ( is_founds[j] == 0 ) { 
        if ( is_new[mytid] == 0 ) {
          is_new[mytid] = 1;
        }
      }
    }
  }
  for ( int i = 0; i < nT; i++ ) { 
    if ( is_new[i] != 0 ) { need_sequential_put = true; }
  }
  if ( need_sequential_put ) { 
    for ( unsigned int i = 0; i < nkeys; i++ ) {
      if ( is_founds[i] == 0 ) {
        VALTYPE oldval;
        int num_probes; // TODO P2 Should do this properly
        status = q_rhashmap_put(hmap, keys[i], vals[i], update_type,
            &oldval, &num_probes);
        cBYE(status);
        // By definition, these keys don't exist and hence oldval == 0
        if ( oldval != 0 ) { go_BYE(-1); }
        num_new++;
      }
    }
    // TODO P1: Should return num_new as diagnostic information
    if ( num_new == 0 ) { go_BYE(-1); }
  }
BYE:
  free_if_non_null(is_new);
  return status;
}
