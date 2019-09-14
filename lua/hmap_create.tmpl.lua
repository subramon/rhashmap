return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_utils.h"
#include "hmap_common.h"
#include "_hmap_types.h"

extern hmap_t *
${fn}(
      size_t minsize
        );

]],
definition = [[

#include "_${fn}.h"
/*
 * hmap_create: construct a new hash table.
 * Number of buckets is larger of input value and pre-defined value
 */
hmap_t *
${fn}(
      size_t minsize
        )
{
  int status = 0;
  hmap_t *ptr_hmap = NULL;

  ptr_hmap = calloc(1, sizeof(hmap_t));
  return_if_malloc_failed(ptr_hmap);

  ptr_hmap->size = ptr_hmap->minsize = MAX(minsize, HASH_INIT_SIZE);


  ptr_hmap->bkts = calloc(ptr_hmap->size, sizeof(bkt_t)); 
  return_if_malloc_failed(ptr_hmap->bkts);

  ptr_hmap-> divinfo = fast_div32_init(ptr_hmap->size);
  ptr_hmap->hashkey ^= random() | (random() << 32);
BYE:
  if ( status != 0 ) { 
    if ( ptr_hmap != NULL ) { 
      free_if_non_null(ptr_hmap->bkts);
    }
    free_if_non_null(ptr_hmap);
    return NULL; 
  }
  else  {
    return ptr_hmap;
  }
}
]]
}

