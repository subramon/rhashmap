/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */
#include "q_rhashmap.h"
//---------------------------------------
static uint64_t
RDTSC(
    void
    )
{
  unsigned int lo, hi;
  asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}
//----------------------------------------------------------
static int
test_multi_set(
    void
    )
{
  int status = 0;
  //C \section{Test multi-set}
  //C \label{multi_set}
  int np; // number of probes
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  int nkeys         = 1048576;
  int n_unique_keys = 1024;
  q_rhashmap_t *hmap = NULL;
  VALTYPE oldval;
  // KEYTYPE key = 123;
  uint64_t t_stop, t_start = RDTSC();
  int update_type = Q_RHM_SET;
  uint8_t *is_founds = NULL;
  uint32_t *locs = NULL;


  locs = calloc(nkeys, sizeof(uint32_t));
  return_if_malloc_failed(locs);
  keys = calloc(nkeys, sizeof(KEYTYPE));
  return_if_malloc_failed(keys);
  vals = calloc(nkeys, sizeof(VALTYPE));
  return_if_malloc_failed(vals);
  hashes = calloc(nkeys,  sizeof(uint32_t));
  return_if_malloc_failed(hashes);
  is_founds = calloc(nkeys,  sizeof(uint8_t));
  return_if_malloc_failed(is_founds);

  //C \begin{itemize}
  //C \item Create hmap.
  hmap = q_rhashmap_create(0); if ( hmap == NULL ) { go_BYE(-1); }

  //C \item Create N random keys into it, all of whom have value 1.
  //C The keys need not be unique
  for ( int i = 0; i < nkeys; i++ ) {
    keys[i] = ( random() % n_unique_keys  ) + 1;
    vals[i] = 1;
  }
  //C \item Put these keys into the hmap using put()
  for ( int i = 0; i < nkeys; i++ ) {
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval, &np);
    cBYE(status);
  }
  //C \item Update value of all keys to 2. 
  for ( int i = 0; i < nkeys; i++ ) { vals[i] = 2; }
  //C \item Create hashes for all the keys
  status = q_rhashmap_mk_hash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  //C \item Initialize arrray locs, the first probe location for each key
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  //C \item Use putn() to update keys in one call (instead of a loop)
  //C Notice that because of
  //C non-uniqueness, the same key may be written to more than once. 
  //C However, all writes have the same value.
  status = q_rhashmap_putn(hmap, update_type, keys, hashes, locs,
      vals, nkeys, is_founds);
  //C \item Verify that is\_found for all keys is true since they have 
  //C already been inserted with value 1
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( !is_founds[i] ) { go_BYE(-1); }
  }
  //C \item Initialize arrray locs, the first probe location for each key
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  //C \item Get values for all keys 
  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  cBYE(status);
  //C Confirm that value for each key is 2
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( vals[i] != 2 ) { go_BYE(-1); }
  }
  //C \item  destroy hmap.
  q_rhashmap_destroy(hmap);
  //C \item  re-create hmap.
  hmap = q_rhashmap_create(0); if ( hmap == NULL ) { go_BYE(-1); }
  //C \item Set keys as \(1, 2, 3, \ldots N\) .
  for ( int i = 0; i < nkeys; i++ ) { keys[i] = i+1; }
  //C \item Set all values to 1.
  for ( int i = 0; i < nkeys; i++ ) { vals[i] = 1; }
  //C Using {\tt put()}, insert all key/value pairs
  for ( int i = 0; i < nkeys; i++ ) { 
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval, &np);
    cBYE(status);
  }
  //C \item Set values for all keys to 2
  for ( int i = 0; i < nkeys; i++ ) { vals[i] = 2; }
  //C \item Create hashes for all keys
  status = q_rhashmap_mk_hash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  //C \item Initialize arrray locs, the first probe location for each key
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  //C \item Use {\tt putn()} and update type = ADD to add 2 to values of all
  //C keys
  update_type = Q_RHM_ADD;
  status = q_rhashmap_putn(hmap, update_type, keys, hashes, locs,
      vals, nkeys, is_founds);
  //C \item Use {\tt getn()} to get all keys that were put in and ascertain value = 1+2.
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  cBYE(status);
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( vals[i] != 1+2 ) { go_BYE(-1); }
  }
  //------------------------------------------------
  t_stop = RDTSC();
  //C \end{itemize}
  fprintf(stdout, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  free_if_non_null(keys);
  free_if_non_null(vals);
  free_if_non_null(locs);
  free_if_non_null(hashes);
  free_if_non_null(is_founds);
  return status;
}
//----------------------------------------------------------
int
main(void)
{
  int status = 0;
  status = test_multi_set(); cBYE(status);
BYE:
  return status;
}
