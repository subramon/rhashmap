return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "fastdiv.h"
#include "_hmap_types.h"

extern int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    void *ptr_val,
    ${caggvaltype} *ptr_oldval,
    int *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"
// rhashmap_insert: internal rhashmap_put(), without the resize.
int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    void * ptr_val,
    ${caggvaltype} *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(
    &key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  int num_probes = *ptr_num_probes;
  register uint32_t size    = ptr_hmap->size;
  register uint64_t divinfo = ptr_hmap->divinfo;
  bool key_updated = false;
  ${caggvaltype} *vals = (${caggvaltype} *)ptr_hmap->vals;

  // 0 is not a valid value for a key, 
  // Note that we do NOT throw an error
  if ( key == 0 ) { return status; }

  uint16_t psl = 0; // default psl of this insert

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
  register uint32_t i = fast_rem32(hash, ptr_hmap->size, ptr_hmap->divinfo);
  for ( ; ; ) { 
    // If there is a key in the bucket.
    if ( keys[i] != 0 ) { 
#ifdef DEBUG
      ASSERT(validate_psl_p(hmap, ptr_bucket, i));
      if ( (bucket->hash == hash) && (ptr_bucket->key == key) ) 
#else
      if ( keys[i] == key ) 
#endif
      {
        where_found = i;
        key_updated = true;
        // START: do the prescribed update 
        ptr_hmap->keys[i] = key;
        ptr_hmap->hashes[i] = hash;
        ${code_for_update};
        
        // STOP: do the prescribed update 
        break;
      }
      // We found a "rich" bucket.  Capture its location.
      if ( ptr_hmap->psls[i] > psl ) { 
         // Place our key-value pair by swapping the "rich"
         // bucket with our entry.  Copy the structures.
        uint16_t swap_psl  = ptr_hmap->psls[i];
        uint16_t swap_hash = ptr_hmap->hashes[i];
        uint16_t swap_key  = ptr_hmap->keys[i];
        // TODO swap_val = XXXX;
        ptr_hmap->psls[i]   = psl;
        ptr_hmap->hashes[i] = hash;
        ptr_hmap->keys[i]  = key;
        // TODO handle val as well
        psl = swap_psl;
        hash = swap_hash;
        key  = swap_key;
        // TODO handle val as well
      }
      entry.psl++;
      /* Continue to the next bucket. */
#ifdef DEBUG
      ASSERT(validate_psl_p(hmap, bucket, i));
#endif
      num_probes++;
      i = fast_rem32(i + 1, size, divinfo);
    }
    else {
      break;
    }
  }
  if ( !key_updated ) {
    // Found a free bucket: insert the entry.
    ptr_hmap->psls[i]   = psl;
    ptr_hmap->hashes[i] = hash;
    ptr_hmap->keys[i]   = key;
    // TODO handle val as well
    ptr_hmap->nitems++;
  }

#ifdef DEBUG
  ASSERT(validate_psl_p(hmap, bucket, i));
#endif
  *ptr_num_probes = num_probes;
  return status;
}
]],
}
