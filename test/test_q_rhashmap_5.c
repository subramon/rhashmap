/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */
#include "q_rhashmap.h"
#include "invariants.h"
#include <omp.h>
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
test_grow_hmap_with_putn(
    void
    )
{
  int status = 0;
  //C \section{Test hmap size increase with putn}
  //C \label{hmap_size_increase_with_putn}
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  int nkeys         = 1048576;
  q_rhashmap_t *hmap = NULL;
  uint64_t t_stop, t_start = RDTSC();
  int update_type = Q_RHM_SET;
  uint8_t *is_founds = NULL;
  uint32_t *locs = NULL;
  uint8_t *tids = NULL;
  int nT = omp_get_num_threads();


  locs = calloc(nkeys, sizeof(uint32_t));
  return_if_malloc_failed(locs);
  tids = calloc(nkeys, sizeof(uint32_t));
  return_if_malloc_failed(tids);
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

  //C \item Create keys/vals` as \(1, 2, \ldots N\)
  for ( int i = 0; i < nkeys; i++ ) {
    keys[i] = i + 1; 
    vals[i] = i + 1;
  }
  //C \item Create array hashes for all the keys.
  status = q_rhashmap_mk_hash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  //C \item Using array hashes, create arrray locs, 
  //C the first probe location for each key.
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, locs);
  cBYE(status);
  status = q_rhashmap_mk_tid(hashes, nkeys, hmap->size, tids);
  cBYE(status);
  //C \item Put these keys/vals into the hmap using putn()
  status = q_rhashmap_putn(hmap, update_type, keys, hashes, locs,
      tids, nT, vals, nkeys, is_founds);
  status = invariants(hmap); cBYE(status);
  //C \item Get values for all keys  using {\tt get()}
  for ( int i = 0; i < nkeys; i++ ) { 
    VALTYPE val;
    bool is_found;
    status = q_rhashmap_get(hmap, i+1, &val, &is_found); cBYE(status);
    if ( val != i+1 ) { go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
  }
  status = invariants(hmap); cBYE(status);
  //C \item Using array hashes, create arrray locs, 
  //C the first probe location for each key. Note that we repeat this
  //C because even though keys and hashes have not changed, the size
  //C of the hash table has changed because of putn above
  status = q_rhashmap_mk_hash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  status = q_rhashmap_mk_loc(hashes, nkeys, hmap->size, locs);
  cBYE(status);
  status = q_rhashmap_mk_tid(hashes, nkeys, hmap->size, tids);
  cBYE(status);
  //C \item Get values for all keys using {\tt getn()}
  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  cBYE(status);
  status = invariants(hmap); cBYE(status);
  //C Confirm that value for each key is 1
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( vals[i] != i+1 ) { go_BYE(-1); }
  }
  //C \item  destroy hmap.
  q_rhashmap_destroy(hmap);
  //------------------------------------------------
  t_stop = RDTSC();
  //C \end{itemize}
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
  status = test_grow_hmap_with_putn(); cBYE(status);
BYE:
  return status;
}
