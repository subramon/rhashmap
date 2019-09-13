return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_hmap_resize.h"
#include "_calc_new_size.h"
extern int
${fn}(
    hmap_t *ptr_hmap,
    ${ckeytype} key,
    bool *ptr_is_found,
    val_t *ptr_val,
    uint64_t *ptr_num_probes
    );

  ]],
definition = [[
#include "_${fn}.h"
int
${fn}(
    hmap_t *ptr_hmap,
    ${ckeytype} key,
    bool *ptr_is_found,
    val_t *ptr_val,
    uint64_t *ptr_num_probes
    )
{
  int status = 0;
  uint32_t hash = murmurhash3(&key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  register uint32_t size = ptr_hmap->size;
  register uint32_t num_probes = 0; 
  register uint32_t probe_loc; 
  bool decreasing = true, resize; uint32_t newsize;

  memset(ptr_val, '\0', sizeof(val_t));
  register bkt_t *bkts = ptr_hmap->bkts;
  probe_loc = fast_rem32(hash, size, ptr_hmap->divinfo);
  if ( probe_loc >= size ) { go_BYE(-1); }
  for ( ; ; ) { 
    // The same probing logic as in the lookup function.
    if ( bkts[probe_loc].key == 0 ) { 
      *ptr_is_found = false; break; 
    }
    if ( num_probes > bkts[probe_loc].psl ) {
      *ptr_is_found = false; break; 
    }
    if ( bkts[probe_loc].key == key ) {
      *ptr_val = bkts[probe_loc].val; // copy old value
      memset(&(bkts[probe_loc]), '\0', sizeof(bkt_t)); // zero out this one
      *ptr_is_found = true;
      ptr_hmap->nitems--;
      break;
    }
    /* Continue to the next bucket. */
    probe_loc++; if ( probe_loc == size ) { probe_loc = 0; }
    num_probes++;
  }
  if ( !*ptr_is_found ) { return status; }
  // Control comes here means we successfully deleted location probe_loc
  /*
   * The probe sequence must be preserved in the deletion case.
   * Use the backwards-shifting method to maintain low variance.
   */
  for ( unsigned int num_moves = 0; ; num_moves++) {
    bkt_t tmp;
    if ( num_moves == size ) { go_BYE(-1); }
    /*
     * Stop if we reach an empty bucket or hit a key which
     * is in its base (original) location.
     */
    uint32_t next_probe_loc = probe_loc + 1; 
    if ( next_probe_loc == size ) { next_probe_loc = 0; }
    if ( ( bkts[next_probe_loc].key == 0 ) || 
         ( bkts[next_probe_loc].psl == 0 ) ) {
      break;
    }
    bkts[next_probe_loc].psl--; // psl cannot be 0 prior to decrement
    tmp = bkts[probe_loc];
    bkts[probe_loc] = bkts[next_probe_loc];
    bkts[next_probe_loc] = tmp;

    num_probes++;
    probe_loc++; 
    if ( probe_loc == size ) { probe_loc = 0; }
  }
  if ( ptr_hmap->nitems < (double)ptr_hmap->size * LOW_WATER_MARK ) { 
    status = calc_new_size(ptr_hmap->nitems, ptr_hmap->minsize, 
    ptr_hmap->size, decreasing, &newsize, &resize);
    cBYE(status);
    if ( resize ) {
      uint64_t bak = num_probes;
      status = hmap_resize(ptr_hmap, newsize, &bak); cBYE(status);
      num_probes += bak;
    }
  }
BYE:
  *ptr_num_probes += num_probes;
  return status;
}
]]
}
