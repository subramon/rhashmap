return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "fastdiv.h"
#include "_hmap_types.h"

extern int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    uint32_t *ptr_loc,
    int *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"
// find a location to put item in
int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    uint32_t *ptr_loc,
    int *ptr_num_probes
    )
{
  int status = 0;
  register uint32_t hash = murmurhash3(
    &key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  register int num_probes = *ptr_num_probes;
  register uint32_t size    = ptr_hmap->size;
  register uint64_t divinfo = ptr_hmap->divinfo;
  register uint16_t psl = 0; // default psl of this insert
  ${caggvaltype} val;  // note that this is a dummy value

  // 0 is not a valid value for a key, We do NOT throw an error
  if ( key == 0 ) { return status; }
  *ptr_loc = 0;
  *ptr_num_probes = 0;

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
  register bool is_found = false;
  register uint32_t i = fast_rem32(hash, ptr_hmap->size, ptr_hmap->divinfo);
  register ${ckeytype} *keys = ptr_hmap->keys;
  register uint16_t *psls = ptr_hmap->psls;
  register ${caggvaltype} *vals = ptr_hmap->vals;
  for ( ; ; ) { 
    num_probes++;
    if ( keys[i] != 0 ) { // If there is a key in the bucket.
      if ( keys[i] == key ) { // key is me 
        *ptr_loc = i;
        break;
      }
      // key is somebody else
      if ( ptr_hmap->psls[i] > psl ) {
        // We found a "rich" bucket.  Capture its location.
         // Place our key-value pair by swapping the "rich"
         // bucket with our entry.  Copy the structures.
       if ( !is_found ) { 
         *ptr_loc = i; // Important: this is done only once
         is_found = true;
       }
        uint16_t swap_psl  = psls[i];
        ${ckeytype} swap_key  = keys[i];
        ${caggvaltype} swap_val  = vals[i];

        psls[i]   = psl;
        keys[i]   = key;
        vals[i]   = val;
        
        psl = swap_psl;
        key = swap_key;
        val = swap_val;
       
      }
      psl++;
      /* Continue to the next bucket. */
      i = fast_rem32(i + 1, size, divinfo);
    }
    else {
      *ptr_loc = i;
    }
  }
  *ptr_num_probes = num_probes;
  return status;
}
]],
}
