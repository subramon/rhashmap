return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
extern int 
${fn} ( // $hmap_getn
    hmap_t *ptr_hmap,
    int nT,
    ${ckeytype} *keys, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] 
    val_t *vals, // OUTPUT [nkeys] 
    uint32_t nkeys, // INPUT 
    uint8_t *fnds // OUTPUT: whether key found or not. TODO Move to bit
    );
    ]],
definition = [[
#include "_${fn}.h"
int 
${fn}(
    hmap_t *ptr_hmap,
    int nT,
    ${ckeytype} *keys, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys] 
    val_t *vals, // OUTPUT [nkeys] 
    uint32_t nkeys, // INPUT 
    uint8_t *fnds // OUTPUT: whether key found or not. TODO Move to bit
    )
    {
  int status = 0;
  register uint32_t size = ptr_hmap->size;
  int block_size = nkeys / nT;

#pragma omp parallel for schedule(static, 1)
  for ( int tid = 0; tid < nT; tid++ ) {
    uint32_t lb = block_size * tid; 
    uint32_t ub = lb + block_size;
    if ( tid == 0 ) { lb = 0; }
    if ( tid == nT-1 ) { ub = nkeys; }
    register bkt_t *bkts = ptr_hmap->bkts;
    printf("%d: %lu: %lu \n", tid, lb, ub);
    for ( uint32_t j = lb; j < ub; j++ ) {
      register uint32_t num_probes = 0; 
      register uint32_t probe_loc = locs[j];
      memset(&(vals[j]), '\0', sizeof(val_t)); 
      fnds[j] = false;
      register ${ckeytype}  key = keys[j];
      for ( ; ; ) { 
        if ( bkts[probe_loc].key == key ) {
          vals[j] = bkts[probe_loc].val;
          fnds[j] = true;
          break; // found 
        }
        if ( ( bkts[probe_loc].key == 0 ) || 
          ( num_probes > bkts[probe_loc].psl ) ) { 
            printf("not found \n");
            break; // not found
          }
          num_probes++;
          probe_loc++; if ( probe_loc == size ) { probe_loc = 0; }
        }
      }
    }
  return status;
}
]],
}
