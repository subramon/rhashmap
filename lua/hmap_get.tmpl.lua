return require 'Q/UTILS/lua/code_gen' {
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
extern int ${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key,
    ${cvaltype} *ptr_val,
    bool *ptr_is_found,
    uint64_t *ptr_num_probes
    );
    ]],
definition = [[
#include "_${fn}.h"
extern int ${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key,
    ${cvaltype} *ptr_val,
    bool *ptr_is_found,
    uint64_t *ptr_num_probes
)
/*
 * hmap_get: lookup an value given the key.
 * => If key is present, *ptr_val is set to its associated value
 * and is_found is set to true
 * => If key is absent, *ptr_val is undefined and is_found is set to false
 */
{
  int status = 0;
  uint32_t hash = murmurhash3(&key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  register uint32_t probe_loc = 
    fast_rem32(hash, ptr_hmap->size, ptr_hmap->divinfo);
  *ptr_is_found = false;
  register uint64_t num_probes = 0;

  /*
   * Lookup is a linear probe.
   */
  register uint64_t size    = ptr_hmap->size;
  register ${ckeytype} *keys = ptr_hmap->keys;
  register uint16_t *psls = ptr_hmap->psls;
  for ( ; ; ) {
    if ( num_probes >= size ) { go_BYE(-1); }
    ${ckeytype} this_key = keys[probe_loc];
    if ( this_key == key ) {
      *ptr_val = ptr_hmap->vals[probe_loc];
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
    if ( ( this_key == 0 ) || ( num_probes > psls[probe_loc] ) ) { 
      *ptr_is_found = false;
      break;
    }
    num_probes++;
    /* Continue to the next bucket. */
    probe_loc++; if ( probe_loc == size ) { probe_loc = 0; } 
  }
BYE:
  return status;
}
]],
}
