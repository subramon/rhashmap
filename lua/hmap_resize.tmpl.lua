return require 'Q/UTILS/lua/code_gen'   {
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_hmap_insert.h"
extern int
${fn}(
    hmap_t *hmap, 
    size_t newsize,
    uint64_t *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"

int
${fn}(
    hmap_t *ptr_hmap, 
    size_t newsize,
    uint64_t *ptr_num_probes
    )
{
  int status = 0;
  if ( ptr_hmap == NULL ) { go_BYE(-1); }
  const size_t oldsize = ptr_hmap->size;
  const size_t nitems  = ptr_hmap->nitems;
  uint64_t num_probes = 0;
  register bkt_t *bkts = ptr_hmap->bkts;

  // some obvious logical checks
  if ( ( newsize <= 0 ) || ( newsize >= UINT_MAX ) )  { go_BYE(-1); }
  if ( newsize < (uint32_t)(HIGH_WATER_MARK * (double)nitems) ) { 
    go_BYE(-1); 
  }
  ptr_hmap->bkts = calloc(sizeof(bkt_t), newsize);
  ptr_hmap->size    = newsize;
  ptr_hmap->nitems  = 0;

   // generate a new hash key/seed every time we resize the hash table.
  ptr_hmap->divinfo = fast_div32_init(newsize);
  ptr_hmap->hashkey ^= random() | (random() << 32);

  for ( uint32_t i = 0; i < oldsize; i++) {
    bool updated = false;
    if ( bkts[i].key == 0 ) { continue; } // skip empty slots
    val_t oldval; // just for function signature match 
    hmap_insert(ptr_hmap, bkts[i].key, bkts[i].val, &oldval, &updated, &num_probes);
    if ( updated ) { go_BYE(-1); }
  }
  free_if_non_null(bkts);
  *ptr_num_probes += num_probes;
BYE:
  return status;
}
]],
}
