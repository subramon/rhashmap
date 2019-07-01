/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */
#include "_q_rhashmap.h"
#include "_invariants.h"
#define VALTYPE  int64_t
#define KEYTYPE uint64_t
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
test_rand_multi_set(
    void
    )
{
  int status = 0;
  //C \section{Test multi-set}
  //C \label{multi_set}
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  q_rhashmap_I8_I8_t *hmap = NULL;
  uint64_t t_stop, t_start = RDTSC();
  int update_type = Q_RHM_SET;
  uint8_t *is_founds = NULL;
  uint32_t *locs = NULL;
  uint8_t *tids = NULL;
  int nT = omp_get_num_threads();
  int key_ub = 1024; // gets multiplied by 2 in every iteration
  int num_iters = 20; 
  int num_keys  = 65536;

  srandom(123456789);
  locs = calloc(num_keys, sizeof(uint32_t));
  return_if_malloc_failed(locs);
  tids = calloc(num_keys, sizeof(uint8_t));
  return_if_malloc_failed(tids);
  keys = calloc(num_keys, sizeof(KEYTYPE));
  return_if_malloc_failed(keys);
  vals = calloc(num_keys, sizeof(VALTYPE));
  return_if_malloc_failed(vals);
  hashes = calloc(num_keys,  sizeof(uint32_t));
  return_if_malloc_failed(hashes);
  is_founds = calloc(num_keys,  sizeof(uint8_t));
  return_if_malloc_failed(is_founds);

  //C \begin{itemize}
  //C \item Create hmap.
  hmap = q_rhashmap_create(0); if ( hmap == NULL ) { go_BYE(-1); }

  //C \item Do the following num\_iters times
  //C \begin{itemize}
  for ( int iters = 1; iters < num_iters; iters++ ) {
    //C \item Create random keys in range 1, \ldots key\_ub
    //C The keys need not be unique. Values are irrelevant.
    for ( int i = 0; i < num_keys; i++ ) {
      keys[i] = ( random() % key_ub  ) + 1;
      vals[i] = 1;
    }
    //C \item Put these keys into the hmap using putn()
    //C \begin{itemize}
    for ( int i = 0; i < num_keys; i++ ) { vals[i] = 2; }
    //C \item Create hashes for all the keys
    status = q_rhashmap_mk_hash_I8(keys, num_keys, hmap->hashkey, hashes);
    cBYE(status);
    //C \item Initialize arrray locs, the first probe location for each key
    status = q_rhashmap_mk_loc(hashes, num_keys, hmap->size, locs);
    //C \item Initialize arrray tids, for paralelization 
    status = q_rhashmap_mk_tid(hashes, num_keys, nT, tids);
    //C \item Use putn() to update keys in one call (instead of a loop)
    status = q_rhashmap_putn_I8_I8(hmap, update_type, keys, hashes, locs,
        tids, nT, vals, num_keys, is_founds);
    cBYE(status);
    status = invariants(hmap); cBYE(status);
    //C \end{itemize}
    //C Confirm that all keys are found using get
    for ( int i = 0; i < num_keys; i++ ) { 
      VALTYPE val; bool is_found;
      status = q_rhashmap_get_I8_I8(hmap, keys[i], &val, &is_found); cBYE(status);
      if ( !is_found ) { go_BYE(-1); }
    }
    //C \item Get values for all keys using getn
    status = q_rhashmap_mk_hash_I8(keys, num_keys, hmap->hashkey, hashes);
    status = q_rhashmap_mk_loc(hashes, num_keys, hmap->size, locs);
    status = q_rhashmap_getn_I8_I8(hmap, keys, hashes, locs, vals, num_keys);
    cBYE(status);
    status = invariants(hmap); cBYE(status);
    //C Confirm that all keys are found
    for ( int i = 0; i < num_keys; i++ ) { 
      if ( vals[i] == 0 ) { go_BYE(-1); }
    }
    key_ub *= 2;
    //C \end{itemize}
    fprintf(stdout, "nitems = %d, size = %d \n",
        (int)hmap->nitems, (int)hmap->size);
  }
  //C \item  destroy hmap.
  q_rhashmap_destroy(hmap);
  status = invariants(hmap); cBYE(status);
  //C \end{itemize}
  //------------------------------------------------
  t_stop = RDTSC();
  fprintf(stdout, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  free_if_non_null(keys);
  free_if_non_null(vals);
  free_if_non_null(locs);
  free_if_non_null(tids);
  free_if_non_null(hashes);
  free_if_non_null(is_founds);
  return status;
}
//----------------------------------------------------------
int
main(void)
{
  int status = 0;
  status = test_rand_multi_set(); cBYE(status);
BYE:
  return status;
}
