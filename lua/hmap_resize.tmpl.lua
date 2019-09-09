return require 'Q/UTILS/lua/code_gen'   {
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_hmap_find_loc.h"
extern int
${fn}(
    hmap_t *hmap, 
    size_t newsize,
    int *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"

int
${fn}(
    hmap_t *ptr_hmap, 
    size_t newsize,
    int *ptr_num_probes
    )
{
  int status = 0;
  ${ckeytype} *oldkeys  = ptr_hmap->keys;
  ${caggvaltype} *oldvals  = ptr_hmap->vals;
  const size_t oldsize = ptr_hmap->size;

  ${ckeytype} *newkeys = NULL;
  ${caggvaltype} *newvals = NULL;
  uint16_t    *newpsls = NULL;


  // some obvious logical checks
  if ( ptr_hmap == NULL ) { go_BYE(-1); }
  if ( ( oldkeys == NULL ) && ( oldsize != 0 ) ) { go_BYE(-1); }
  if ( ( oldvals == NULL ) && ( oldsize != 0 ) ) { go_BYE(-1); }
  if ( ( newsize <= 0 ) || ( newsize >= UINT_MAX ) )  { go_BYE(-1); }
  if ( newsize <= (ptr_hmap->nitems/HIGH_WATER_MARK) ) { go_BYE(-1); }

  free_if_non_null(ptr_hmap->psls);

  // allocate new storage
  newkeys = calloc(newsize, sizeof(${ckeytype}));
  newvals = calloc(newsize, sizeof(${caggvaltype}));
  newpsls   = calloc(newsize, sizeof(uint16_t));

  if ( ( newkeys == NULL ) || ( newvals == NULL ) || 
       ( newpsls == NULL ) ) { go_BYE(-1); }

  ptr_hmap->vals    = newvals;
  ptr_hmap->keys    = newkeys;
  ptr_hmap->psls    = newpsls;
  ptr_hmap->size    = newsize;
  ptr_hmap->nitems  = 0;

   // generate a new hash key/seed every time we resize the hash table.
  ptr_hmap->divinfo = fast_div32_init(newsize);
  ptr_hmap->hashkey ^= random() | (random() << 32);

  for ( uint32_t i = 0; i < oldsize; i++) {
    int num_probes = 0;
    uint32_t loc = 0;
    ${ckeytype} key = ptr_hmap->keys[i];
    if ( key == 0 ) { continue; }  // skip empty slot 
    ${caggvaltype} val = ptr_hmap->vals[i];
    status = hmap_find_loc( ptr_hmap, key, &loc, &num_probes);
    cBYE(status);
    ptr_hmap->keys[loc] = key;
    ptr_hmap->vals[loc] = val;
    // TODO ptr_hmap->psls[loc] = psl;
    *ptr_num_probes += num_probes;
  }
  free_if_non_null(oldkeys);
  free_if_non_null(oldvals);
BYE:
  return status;
}
]],
}
