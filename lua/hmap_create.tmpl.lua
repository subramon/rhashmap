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
//START_FUNC_DECL
hmap_t *
${fn}(
      size_t minsize
        )
{
  int status = 0;
  hmap_t *hmap = NULL;

  hmap->size = hmap->minsize = MAX(minsize, HASH_INIT_SIZE);

  hmap = calloc(1, sizeof(hmap_t));
  return_if_malloc_failed(hmap);

  hmap->psls = calloc(hmap->size, sizeof(uint16_t)); 
  return_if_malloc_failed(hmap->psls);

  hmap->vals = calloc(hmap->size, sizeof(${caggvaltype}));
  return_if_malloc_failed(hmap->vals);
#ifdef DEBUG
  hmap->hashes = calloc(hmap->size, sizeof(uint32_t)); 
  return_if_malloc_failed(hmap->hashes);
#endif
  hmap->keys = calloc(hmap->size, sizeof(${ckeytype}));
  return_if_malloc_failed(hmap->keys);

BYE:
  if ( status != 0 ) { 
    if ( hmap != NULL ) { 
      free_if_non_null(hmap->keys);
      free_if_non_null(hmap->vals);
#ifdef DEBUG
      free_if_non_null(hmap->hashes);
#endif
      free_if_non_null(hmap->psls);
    }
    free_if_non_null(hmap);
    return NULL; 
  }
  else  {
    return hmap;
  }
}
]]
}

