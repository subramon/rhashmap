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


  ptr_hmap->psls = calloc(ptr_hmap->size, sizeof(uint16_t)); 
  return_if_malloc_failed(ptr_hmap->psls);

  ptr_hmap->vals = calloc(ptr_hmap->size, sizeof(${cvaltype}));
  return_if_malloc_failed(ptr_hmap->vals);
  ptr_hmap->keys = calloc(ptr_hmap->size, sizeof(${ckeytype}));
  return_if_malloc_failed(ptr_hmap->keys);

  ptr_hmap-> divinfo = fast_div32_init(ptr_hmap->size);
BYE:
  if ( status != 0 ) { 
    if ( ptr_hmap != NULL ) { 
      free_if_non_null(ptr_hmap->keys);
      free_if_non_null(ptr_hmap->vals);
      free_if_non_null(ptr_hmap->psls);
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

