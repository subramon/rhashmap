return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_hmap_put.h"
// TODO What about returning cnt?
extern int 
${fn}( // hmap_putn
    hmap_t *ptr_hmap,  // INPUT
    ${ckeytype} *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] -- starting point for probe
    uint8_t *tids, // INPUT [nkeys] -- thread who should work on it
    int nT,
    val_t *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *fnds, // OUTPUT [nkeys] Whether found or not. TODO: Change to bits
    uint32_t *ptr_num_new, // OUTPUT for diagnostics
    uint64_t *ptr_num_probes // OUTPUT for diagnostics
    );
    ]],
definition = [[
#include "_${fn}.h"
// Given 
// (1) a set of keys 
// (2) hash for each key
// (3) value for each key
// Update as follows. If j^{th} key found, then 
// (A) set is_found[j]  to true
// (B) update value with new value provided (either set or add)
// Else, set is_found[j] to false
int 
${fn}( // hmap_putn
    hmap_t *ptr_hmap,  // INPUT
    ${ckeytype} *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] -- starting point for probe
    uint8_t *tids, // INPUT [nkeys] -- thread who should work on it
    int nT, // number of threads
    val_t *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *fnds, // OUTPUT [nkeys bits] TODO: Change to bits
    uint32_t *ptr_num_new, // OUTPUT for diagnostics
    uint64_t *ptr_num_probes // OUTPUT for diagnostics
    )
{
  int status = 0;
  bool *is_new = NULL;
  register uint32_t size    = ptr_hmap->size;
  uint64_t *all_num_probes = 0;

  is_new = malloc(nT * sizeof(bool));
  return_if_malloc_failed(is_new);
  for ( int i = 0; i < nT; i++ ) { is_new[i] = false; }
  all_num_probes = malloc(nT * sizeof(uint64_t));
  return_if_malloc_failed(all_num_probes);
  for ( int i = 0; i < nT; i++ ) { all_num_probes[i] = 0; }

#pragma omp parallel
  {
    // TODO P3 Can I avoid get_thread_num() in each iteration?
    register uint32_t mytid = omp_get_thread_num();
    uint32_t num_probes = 0; 
    for ( uint32_t j = 0; j < nkeys; j++ ) {
      // Following so that 2 threads don't deal with same key
      if ( tids[j] != mytid )  { continue; }
      register ${ckeytype} key = keys[j];
      register bkt_t *bkts = ptr_hmap->bkts;
      uint32_t probe_loc = locs[j]; // fast_rem32(hash, size, divinfo);
      fnds[j] = 0;

      for ( ; ; ) { // search until found 
        if ( bkts[probe_loc].key == key ) {
          // start code for update
          // stop  code for update
          fnds[j] = 1;
          break; 
        }
        if ( ( bkts[probe_loc].key == 0 ) || 
            ( num_probes > bkts[probe_loc].psl ) ) { 
          // not found
          fnds[j] = 0; 
          break; 
        }
        num_probes++;
        probe_loc++; if ( probe_loc == size ) { probe_loc = 0; }
      }
      if ( fnds[j] == 0 ) { 
        if ( is_new[mytid] == 0 ) {
          is_new[mytid] = 1;
        }
      }
    }
    all_num_probes[mytid] = num_probes;
  }
  // Find out if new keys were provided in the above loop
  bool need_sequential_put = false;
  for ( int tid = 0; tid < nT; tid++ ) { 
    if ( is_new[tid] != 0 ) { need_sequential_put = true; }
  }
  // If so, we have no choice but to put these in sequentially
  // TODO P2: Currently, we are scannning the entire list of keys, 
  // looking for the ones to add. Ideally, each thread should keep 
  // a list of keys to be added and we should just scan that list.
  uint64_t num_probes = 0;
  uint32_t num_new    = 0;
  if ( need_sequential_put ) { 
    for ( unsigned int i = 0; i < nkeys; i++ ) {
      if ( fnds[i] == 0 ) {
        val_t oldval;
        bool is_updated; // to match function signature
        status = hmap_put(ptr_hmap, keys[i], vals[i], 
            &oldval, &is_updated, &num_probes);
        cBYE(status);
        num_new++;
      }
    }
    if ( num_new == 0 ) { go_BYE(-1); }
  }
  for ( int i = 0; i < nT; i++ ) { 
    num_probes += all_num_probes[i];
  }
  *ptr_num_probes = num_probes;
  *ptr_num_new    = num_new;
BYE:
  free_if_non_null(is_new);
  free_if_non_null(all_num_probes);
  return status;
}
]]
}
