return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_type_${qkeytype}.h"
extern void
${fn}(
    hmap_${qkeytype}_t *ptr_hmap
    );
]],
definition = [[
#include "_${fn}.h"
void
${fn}(
    hmap_${qkeytype}_t *ptr_hmap
    )
{
  if ( ptr_hmap == NULL ) { return; }
  free_if_non_null(ptr_hmap->buckets);
  free_if_non_null(ptr_hmap->vals);
  ptr_hmap->size = 0;
  ptr_hmap->nitems = 0;
  ptr_hmap->divinfo = 0;
  ptr_hmap->buckets = 0;
  ptr_hmap->hashkey = 0;
  ptr_hmap->minsize = 0;
  free_if_non_null(ptr_hmap);
}
]],
}
