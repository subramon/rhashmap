return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
#include "_calc_new_size.h"
#include "_hmap_resize.h"
#include "_hmap_insert.h"
extern int
hmap_put(
    hmap_t *hmap, 
    ${ckeytype}  key, 
    ${cinvaltype}  val, 
    ${caggvaltype} *ptr_oldval,
    int *ptr_num_probes
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
    hmap_t *hmap, 
    ${ckeytype}  key, 
    ${cinvaltype}  val, 
    ${caggvaltype} *ptr_oldval,
    int *ptr_num_probes
    )
{
  int status = 0;
  uint32_t newsize; bool resize, decreasing = false;

  status = calc_new_size(hmap->nitems, hmap->minsize, hmap->size, 
      decreasing, &newsize, &resize);
  if ( resize ) { 
    status = hmap_resize(hmap, newsize, ptr_num_probes); cBYE(status);
  }
  status = hmap_insert(hmap, key, val, ptr_oldval, ptr_num_probes);
  cBYE(status);
BYE:
  return status;
}
]],
}
