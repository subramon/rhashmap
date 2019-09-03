return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "fastdiv.h"
#include "_hmap_types.h"

extern int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    ${caggvaltype} val,
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
    ${cvaltype} val,
    ${caggvaltype} *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(
    &key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  bkt_type *ptr_bucket, entry;
  uint32_t i;
  int num_probes = 0;
  register uint32_t size    = ptr_hmap->size;
  register uint64_t divinfo = ptr_hmap->divinfo;
  bool key_updated = false;
  ${caggvaltype} *vals = (${caggvaltype} *)ptr_hmap->vals;

  // 0 is not a valid value for a key, 
  // Note that we do NOT throw an error
  if ( key == 0 ) { return status; }

  // Setup the bucket entry.
  entry.key   = key;
#ifdef DEBUG
  entry.hash  = hash;
#endif
  entry.psl   = 0;
  *ptr_oldval = 0;

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
  i = fast_rem32(hash, ptr_hmap->size, ptr_hmap->divinfo);
  uint32_t where_found;
  for ( ; ; ) { 
    ptr_bucket = &(ptr_hmap->buckets[i]);
    // If there is a key in the bucket.
    if ( ptr_bucket->key ) {
#ifdef DEBUG
      ASSERT(validate_psl_p(hmap, ptr_bucket, i));
      if ( (bucket->hash == hash) && (ptr_bucket->key == key) ) 
#else
      if ( ptr_bucket->key == key ) 
#endif
      {
        where_found = i;
        key_updated = true;
        // do the prescribed update 
        *ptr_oldval = vals[i];
        ${code_for_update};
        break;
      }
      // We found a "rich" bucket.  Capture its location.
      if (entry.psl > ptr_bucket->psl) {
        bkt_type tmp;
        /*
         * Place our key-value pair by swapping the "rich"
         * bucket with our entry.  Copy the structures.
         */
        tmp = entry;
        entry = *ptr_bucket;
        *ptr_bucket = tmp;
        // TODO Set val 
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
    *ptr_bucket = entry; // copy
    vals[where_found] = val; 
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
