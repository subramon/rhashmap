/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */
#include "_q_rhashmap_I8_I8.h"
#include "_q_rhashmap_mk_hash_I8.h"
#include "q_rhashmap_mk_loc.h"
#include "q_rhashmap_mk_tid.h"
#include "_invariants_I8_I8.h"
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
test_putn_perf(
    void
    )
{
  int status = 0;
  //C \section{Test putn performance}
  //C \label{putn_perf}
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  q_rhashmap_I8_I8_t *hmap = NULL;
  int update_type = Q_RHM_ADD;
  uint8_t *is_founds = NULL;
  uint32_t *locs = NULL;
  uint8_t *tids = NULL;
  int nT = omp_get_num_threads();
  int num_iters = 64; 
  int num_keys  = 65536;
  int n = 1048576;
  uint64_t t_total = 0, t_start, t_stop;

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
  hmap = q_rhashmap_create_I8_I8(0); if ( hmap == NULL ) { go_BYE(-1); }

  //C \item Create keys 1, 2, ... n = 1048576, values are irrelevant.
  for ( int i = 0; i < num_keys; i++ ) {
    keys[i] = i+1;
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
  status = invariants_I8_I8(hmap); cBYE(status);
  //C \end{itemize}
  //C%-------------------------------------------------------
  //C \item Do the following num\_iters times
  //C \begin{itemize}
  for ( int iters = 1; iters < num_iters; iters++ ) {
    //C \item Select lb, ub randomly from 1 to n, where lb < ub.
    //C Set the value of n keys randomly from lb to ub (there will be dupes)
    //C Values once again are irrelevant
    int lb, ub;
    for ( ; ; ) {
      lb = random() % n;
      ub = random() % n;
      if ( lb > ub ) { int swap = lb; lb = ub; ub = swap; }
      if ( ( ub - lb ) >= 10 ) { break; }
    }
    for ( int i = 0; i < num_keys; i++ ) {
      keys[i] = (random() % ( ub - lb )) + 1;
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
    t_start = RDTSC();
    status = q_rhashmap_putn_I8_I8(hmap, update_type, keys, hashes, locs,
        tids, nT, vals, num_keys, is_founds);
    cBYE(status);
    t_stop = RDTSC();
    t_total += (t_stop - t_start);
    status = invariants_I8_I8(hmap); cBYE(status);
    //C \end{itemize}
  }
  //C \end{itemize}
  //C \item  destroy hmap.
  q_rhashmap_destroy_I8_I8(hmap);
  status = invariants_I8_I8(hmap); cBYE(status);
  //C \end{itemize}
  //------------------------------------------------
  t_stop = RDTSC();
  fprintf(stdout, "Iterations = %d, Keys = %d, Time = %" PRIu64 "\n",
      num_iters, n, t_total);
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
  status = test_putn_perf(); cBYE(status);
BYE:
  return status;
}
