// TODO: Any advantages to size being a prime number?

/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.

 This code was initially forked from the following. Subsequent to that, 
 significant modifications have been made and new functionality added,
 bugs fixes, ....

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

#include "_calc_new_size.h"
#include "_validate_psl_p.h"
#include "_q_rhashmap___KV__.h"

//------------------------------------------------------
int 
q_rhashmap_getn___KV__(
    q_rhashmap___KV___t *hmap, // INPUT
    __KEYTYPE__ *keys, // INPUT: [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] 
    __VALTYPE__ *vals, // OUTPUT [nkeys] 
    uint32_t nkeys // INPUT 
    // TODO P4 we won't do is_found for the first implementation
    )
{
  int status = 0;
  int chunk_size = 1024;
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t j = 0; j < nkeys; j++ ) {
    uint32_t n = 0; 
    uint32_t i = locs[j];
#ifdef DEBUG
    uint32_t hash = hashes[j];
#endif
    q_rh_bucket___KV___t *bucket = NULL;
    vals[j]     = 0;

    for ( ; ; ) { 
      bucket = &hmap->buckets[i];
#ifdef DEBUG
      ASSERT(validate_psl_p(hmap, bucket, i));
      if ( ( bucket->hash == hash ) && ( bucket->key == keys[j] ) ) 
#else
      if ( bucket->key == keys[j] ) 
#endif
      {
        vals[j] = bucket->val;
        break; // found 
      }
      if (!bucket->key || n > bucket->psl) {
        break; // not found
      }
      n++;
      i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    }
  }
  return status;
}
/*
 * rhashmap_del: remove the given key and return its value.
 *
 * => If key was present, return its associated value; otherwise NULL.
 */
int
q_rhashmap_del___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ *ptr_oldval,
    bool *ptr_is_found
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(__KEYTYPE__), hmap->hashkey);
  uint32_t n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
  q_rh_bucket___KV___t *bucket;
  *ptr_oldval = 0;
  bool decreasing = true, resize; uint32_t newsize;

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
#ifdef DEBUG
  ASSERT(validate_psl_p(hmap, bucket, i));
  if ( ( bucket->hash != hash ) || ( bucket->key != key ) ) 
#else
  if ( bucket->key != key ) 
#endif
  {
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
#ifdef DEBUG
  bucket->hash = 0; 
#endif
  bucket->psl  = 0; 
  hmap->nitems--;

  /*
   * The probe sequence must be preserved in the deletion case.
   * Use the backwards-shifting method to maintain low variance.
   */
  for ( ; ; ) {
    q_rh_bucket___KV___t *nbucket = NULL;

    bucket->key = 0;
    bucket->val  = 0; 
#ifdef DEBUG
    bucket->hash = 0; 
#endif
    bucket->psl  = 0; 

    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    nbucket = &hmap->buckets[i];
#ifdef DEBUG
    ASSERT(validate_psl_p(hmap, nbucket, i));
#endif

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
  status = calc_new_size(hmap->nitems, hmap->minsize, hmap->size, 
      decreasing, &newsize, &resize);
  cBYE(status);
  if ( resize ) {
    status = q_rhashmap_resize(hmap, newsize); cBYE(status);
  }
BYE:
  return status;
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
q_rhashmap_putn___KV__(
    q_rhashmap___KV___t *hmap,  // INPUT
    int update_type, // INPUT
    __KEYTYPE__ *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] -- starting point for probe
    uint8_t *tids, // INPUT [nkeys] -- thread who should work on it
    int nT,
    __VALTYPE__ *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *is_founds // OUTPUT [nkeys bits] TODO: Change from byte to bit 
    )
{
  int status = 0;
  int *is_new = NULL;
  register uint32_t hmap_size    = hmap->size;
  register uint64_t hmap_divinfo = hmap->divinfo;
  // quick sanity check 
  switch ( update_type ) { 
    case Q_RHM_SET : case Q_RHM_ADD : break;
    default: go_BYE(-1); break;
  }
  is_new = malloc(nT * sizeof(int));
  return_if_malloc_failed(is_new);
  for ( int i = 0; i < nT; i++ ) { is_new[i] = 0; }

#pragma omp parallel
  {
    // TODO P3 Can I avoid get_thread_num() in each iteration?
    register uint32_t mytid = omp_get_thread_num();
    for ( uint32_t j = 0; j < nkeys; j++ ) {
      // Following so that 2 threads don't deal with same key
      if ( tids[j] != mytid )  { continue; }
#ifdef DEBUG
      register uint32_t hash = hashes[j];
#endif
      register q_rh_bucket___KV___t *buckets = hmap->buckets;
      register __KEYTYPE__ key = keys[j];
      register __VALTYPE__ val = vals[j];
      uint32_t i = locs[j]; // fast_rem32(hash, hmap_size, hmap_divinfo);
      is_founds[j] = 0;
      uint32_t n = 0; 

      for ( ; ; ) { // search until found 
        q_rh_bucket___KV___t *bucket = buckets + i;
#ifdef DEBUG
        ASSERT(validate_psl_p(hmap, bucket, i)); 
        if ( ( bucket->hash == hash ) && ( bucket->key == key ) )
#else
        if ( bucket->key == key )
#endif
        {
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
  // Find out if new keys were provided in the above loop
  bool need_sequential_put = false;
  for ( int i = 0; i < nT; i++ ) { 
    if ( is_new[i] != 0 ) { need_sequential_put = true; }
  }
  // If so, we have no choice but to put these in sequentially
  // TODO P2: Currently, we are scannning the entire list of keys, 
  // looking for the ones to add. Ideally, each thread should keep 
  // a list of keys to be added and we should just scan that list.
  int num_new = 0;
  if ( need_sequential_put ) { 
    for ( unsigned int i = 0; i < nkeys; i++ ) {
      if ( is_founds[i] == 0 ) {
        __VALTYPE__ oldval;
        int num_probes; // TODO P2 Should do this properly
        status = q_rhashmap_put___KV__(hmap, keys[i], vals[i], update_type,
            &oldval, &num_probes);
        cBYE(status);
        /* Following has been commented out because it is a wrong check
          By definition, these keys don't exist and hence oldval == 0
          if ( oldval != 0 ) { go_BYE(-1); }
        */
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
