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

#include "_q_rhashmap___KV__.h"

static int __attribute__((__unused__))
validate_psl_p(
    q_rhashmap___KV___t *hmap, 
    const q_rh_bucket___KV___t *bucket, 
    uint32_t i
    )
{
  uint32_t base_i = fast_rem32(bucket->hash, hmap->size, hmap->divinfo);
  uint32_t diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
  return bucket->key == 0 || diff == bucket->psl;
}

/* Checks whether resize is needed. If so, calculates newsize */
/* Resize needed when occupancy is too high or too low */
static int
calc_new_size(
    uint32_t nitems, 
    uint32_t minsize, 
    uint32_t size, 
    bool decreasing, 
    /* true =>  just added an element and are concerned about sparsity
     * false=> just added an element and are concerned about denseness
    */
    uint32_t *ptr_newsize,
    bool *ptr_resize
    )
{
  int status = 0;
  *ptr_resize = false;
  *ptr_newsize = 0;
  uint32_t threshold;
  if ( decreasing ) { 
    /*
     * If the load factor is less than threshold, then shrink by
     * halving the size, but not more than the minimum size.
     */
    threshold = (uint32_t)(LOW_WATER_MARK * size);
    if ( ( nitems > minsize ) && ( nitems < threshold ) ) {
      *ptr_resize = true;
      *ptr_newsize = MAX(size >> 1, minsize);
    }
  }
  else {
    /*
     * If the load factor is more than the threshold, then resize.
     */
    threshold = (uint32_t)(0.85 * (float)size);
    // TODO P4 Clean up the following code 
    if ( nitems > threshold ) { 
      *ptr_resize = true;
      for ( ; nitems > threshold; ) { 
        /*
         * Grow the hash table by doubling its size, but with
         * a limit of MAX_GROWTH_STEP.
         */
       // TODO: P4 Worry about overflow in addition below
        const size_t grow_limit = size + MAX_GROWTH_STEP;
        *ptr_newsize = MIN(size << 1, grow_limit);
        threshold = (uint32_t)(0.85 * *ptr_newsize);
      }
    }
  }
  return status;
}

/*
 * q_rhashmap_get: lookup an value given the key.
 *
 * => If key is present, *ptr_val is set to its associated value
 * and is_found is set to true
 * => If key is absent, *ptr_val is set to 0
 * and is_found is set to false
 */
int 
q_rhashmap_get___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__  key,
    __VALTYPE__ *ptr_val,
    bool *ptr_is_found
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(__KEYTYPE__), hmap->hashkey);
  uint32_t n = 0; 
  uint32_t i = fast_rem32(hash, hmap->size, hmap->divinfo);
  q_rh_bucket___KV___t *bucket = NULL;
  *ptr_is_found = false;
  *ptr_val      = 0;

  /*
   * Lookup is a linear probe.
   */
  register uint64_t divinfo = hmap->divinfo;
  register uint64_t size    = hmap->size;
  for ( ; ; ) { 
    bucket = &hmap->buckets[i];
    ASSERT(validate_psl_p(hmap, bucket, i));

    if ( ( bucket->hash == hash ) && ( bucket->key == key ) ) {
      *ptr_val = bucket->val;
      *ptr_is_found = true;
      break;
    }

    /*
     * Stop probing if we hit an empty bucket; also, if we hit a
     * bucket with PSL lower than the distance from the base location,
     * then it means that we found the "rich" bucket which should
     * have been captured, if the key was inserted -- see the central
     * point of the algorithm in the insertion function.
     */
    if ( ( !bucket->key ) || ( n > bucket->psl ) ) {
      *ptr_is_found = false;
      break;
    }
    n++;
    /* Continue to the next bucket. */
    i = fast_rem32(i + 1, size, divinfo);
  }
  return status;
}

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
    uint32_t hash = hashes[j];
    q_rh_bucket___KV___t *bucket = NULL;
    vals[j]     = 0;

    for ( ; ; ) { 
      bucket = &hmap->buckets[i];
      ASSERT(validate_psl_p(hmap, bucket, i));

      if ( ( bucket->hash == hash ) && ( bucket->key == keys[j] ) ) {
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
 * rhashmap_insert: internal rhashmap_put(), without the resize.
 */
static int
q_rhashmap_insert(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ val,
    int update_type,
    __VALTYPE__ *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(__KEYTYPE__), hmap->hashkey);
  q_rh_bucket___KV___t *bucket, entry;
  uint32_t i;
  int num_probes = 0;
  register uint32_t size = hmap->size;
  register uint64_t divinfo = hmap->divinfo;
  bool key_updated = false;

  // 0 is not a valid value for a key, TODO P3 Document this better
  // Note that we do NOT throw an error
  if ( key == 0 ) { return status; }

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
  for ( ; ; ) { 
    bucket = &hmap->buckets[i];
    // If there is a key in the bucket.
    if ( bucket->key ) {
      ASSERT(validate_psl_p(hmap, bucket, i));

      // TODO P4 Why are we comparing the hash at all below?
      // If there is a key in the bucket and its you
      if ( (bucket->hash == hash) && (bucket->key == key) ) { 
        key_updated = true;
        // do the prescribed update 
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
        break;
      }
      // We found a "rich" bucket.  Capture its location.
      if (entry.psl > bucket->psl) {
        q_rh_bucket___KV___t tmp;
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
      i = fast_rem32(i + 1, size, divinfo);
    }
    else {
      break;
    }
  }
  if ( !key_updated ) {
    // Found a free bucket: insert the entry.
    *bucket = entry; // copy
    hmap->nitems++;
  }

  ASSERT(validate_psl_p(hmap, bucket, i));
  *ptr_num_probes = num_probes;
BYE:
  return status;
}

static int
q_rhashmap_resize(
    q_rhashmap___KV___t *hmap, 
    size_t newsize
    )
{
  int status = 0;
  q_rh_bucket___KV___t *oldbuckets = hmap->buckets;
  const size_t oldsize = hmap->size;
  q_rh_bucket___KV___t *newbuckets = NULL;
  const size_t len = newsize * sizeof(q_rh_bucket___KV___t);
  int num_probes = 0;

  // some obvious logical checks
  if ( ( oldbuckets == NULL ) && ( oldsize != 0 ) ) { go_BYE(-1); }
  if ( ( oldbuckets != NULL ) && ( oldsize == 0 ) ) { go_BYE(-1); }
  if ( ( newsize <= 0 ) || ( newsize >= UINT_MAX ) )  { go_BYE(-1); }
  if ( newsize < hmap->nitems ) { go_BYE(-1); }

  // allocate buckets.  
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
    const q_rh_bucket___KV___t *bucket = &oldbuckets[i];

    /* Skip the empty buckets. */
    if ( !bucket->key ) { continue; }

    __VALTYPE__ oldval; // not needed except for signature
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
q_rhashmap_put___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key, 
    __VALTYPE__ val,
    int update_type,
    __VALTYPE__ *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  uint32_t newsize; bool resize, decreasing = false;

  status = calc_new_size(hmap->nitems, hmap->minsize, hmap->size, 
      decreasing, &newsize, &resize);
  if ( resize ) { 
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
    q_rh_bucket___KV___t *nbucket = NULL;

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
  status = calc_new_size(hmap->nitems, hmap->minsize, hmap->size, 
      decreasing, &newsize, &resize);
  cBYE(status);
  if ( resize ) {
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
q_rhashmap___KV___t *
q_rhashmap_create___KV__(
      size_t size
        )
{
  q_rhashmap___KV___t *hmap = NULL;

  hmap = calloc(1, sizeof(q_rhashmap___KV___t));
  if ( hmap == NULL ) { return NULL; }

  hmap->minsize = MAX(size, HASH_INIT_SIZE);
  if ( q_rhashmap_resize(hmap, hmap->minsize) != 0) {
    free(hmap);
    return NULL;
  }
  if (hmap->buckets == NULL ) { WHEREAMI; return NULL; }
  if (hmap->size    == 0    ) { WHEREAMI; return NULL; }
  return hmap;
}

/*
 * rhashmap_destroy: free the memory used by the hash table.
 *
 * => It is the responsibility of the caller to remove elements if needed.
 */
void
q_rhashmap_destroy___KV__(
    q_rhashmap___KV___t *ptr_hmap
    )
{
  free(ptr_hmap->buckets);
  ptr_hmap->buckets = NULL;
  ptr_hmap->size = 0;
  ptr_hmap->nitems = 0;
  ptr_hmap->divinfo = 0;
  ptr_hmap->buckets = 0;
  ptr_hmap->hashkey = 0;
  ptr_hmap->minsize = 0;

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
  int num_new = 0;
  bool need_sequential_put = false;
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
      register uint32_t hash = hashes[j];
      register q_rh_bucket___KV___t *buckets = hmap->buckets;
      register __KEYTYPE__ key = keys[j];
      register __VALTYPE__ val = vals[j];
      uint32_t i = locs[j]; // fast_rem32(hash, hmap_size, hmap_divinfo);
      is_founds[j] = 0;
      uint32_t n = 0; 

      for ( ; ; ) { // search until found 
        q_rh_bucket___KV___t *bucket = buckets + i;
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
  // Find out if new keys were provided in the above loop
  for ( int i = 0; i < nT; i++ ) { 
    if ( is_new[i] != 0 ) { need_sequential_put = true; }
  }
  // If so, we have no choice but to put these in sequentially
  // TODO P2: Currently, we are scannning the entire list of keys, 
  // looking for the ones to add. Ideally, each thread should keep 
  // a list of keys to be added and we should just scan that list.
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
