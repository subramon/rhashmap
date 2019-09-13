return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "fastdiv.h"
#include "_hmap_types.h"

extern int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    val_t val,
    val_t *ptr_old_val,
    bool *ptr_updated,
    uint64_t *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"
// rhashmap_insert: internal rhashmap_put(), without the resize.
int
${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype} key,
    val_t val,
    val_t *ptr_old_val,
    bool *ptr_updated,
    uint64_t *ptr_num_probes
    )
{
  int status = 0;
  const uint32_t hash = murmurhash3(&key, sizeof(${ckeytype}), 
    ptr_hmap->hashkey);
  register uint32_t probe_loc; // location where we probe
  register uint64_t num_probes = 0;
  register uint32_t size = ptr_hmap->size;
  uint64_t divinfo = ptr_hmap->divinfo;
  *ptr_updated = false;
  memset(ptr_old_val, '\0', sizeof(val_t));

  // 0 is not a valid value for a key, TODO P3 Document this better
  // Note that we do NOT throw an error
  if ( key == 0 ) { return status; }
 
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
   // set up the bucket entry 
  bkt_t entry;
  memset(&entry, '\0', sizeof(bkt_t));
  entry.val = val;
  entry.key = key;
  //-----------
  register bkt_t *bkts  = ptr_hmap->bkts;
  probe_loc = fast_rem32(hash, size, divinfo);
  if ( probe_loc >= size ) { go_BYE(-1); }
  for ( ; ; ) {
    if ( num_probes >= size ) { go_BYE(-1); }
    ${ckeytype} this_key = bkts[probe_loc].key;
    if ( this_key != 0 ) { // If there is a key in the bucket.
      if ( this_key == key ) { 
        *ptr_old_val = bkts[probe_loc].val;
        ${code_for_update} // TODO 
        ptr_hmap->bkts[probe_loc].cnt += 1;
        *ptr_updated = true;
        break;
      }
      //-----------------------
      // We found a "rich" bucket.  Capture its location.
      if ( entry.psl > bkts[probe_loc].psl ) {
        bkt_t tmp;
        tmp = entry;
        entry = bkts[probe_loc];
        bkts[probe_loc] = tmp;
      }
      entry.psl++;
      /* Continue to the next bucket. */
      num_probes++;
      probe_loc++;
      if ( probe_loc == size ) { probe_loc = 0; }
    }
    else {
      bkts[probe_loc] = entry;
      ptr_hmap->bkts[probe_loc].cnt = 0;
      ptr_hmap->nitems++;
      break;
    }
  }
  *ptr_num_probes += num_probes;
BYE:
  return status;
}
]],
}

