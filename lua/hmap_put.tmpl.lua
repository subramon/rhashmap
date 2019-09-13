return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_calc_new_size.h"
#include "_hmap_resize.h"
#include "_hmap_insert.h"
extern int
hmap_put(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key, 
    val_t val, 
    val_t *ptr_oldval,
    bool *ptr_updated,
    uint64_t *ptr_num_probes
    );
    ]],
definition = [[
/*
 * rhashmap_put: insert a value given the key.
 *
 * => If the key is already present, return its associated value.
 * => Otherwise, on successful insert, return the given value.
 */
 #include "_${fn}.h"
int
hmap_put(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key, 
    val_t val, 
    val_t *ptr_oldval,
    bool *ptr_updated,
    uint64_t *ptr_num_probes
    )
{
  int status = 0;
  uint32_t newsize; bool resize = false, decreasing = false;
  ${cvaltype} val;
  uint64_t num_probes = 0;

  if ( ptr_hmap->nitems > (double)ptr_hmap->size * HIGH_WATER_MARK ) {
    status = calc_new_size(ptr_hmap->nitems, ptr_hmap->minsize, 
    ptr_hmap->size, decreasing, &newsize, &resize);
    cBYE(status);
  }
  if ( resize ) { 
    status = hmap_resize(ptr_hmap, newsize, ptr_num_probes); cBYE(status);
  }
  status = hmap_insert(ptr_hmap, key, val, ptr_oldval, ptr_updated, &num_probes);
  cBYE(status);
  *ptr_num_probes += num_probes;
BYE:
  return status;
}
]],
}
