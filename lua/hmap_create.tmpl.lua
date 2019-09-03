return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_utils.h"
#include "hmap_common.h"
#include "_hmap_type_${qkeytype}.h"

extern hmap_${qkeytype}_t *
${fn}(
      size_t minsize,
      size_t sizeof_val
        );

]],
definition = [[

#include "_${fn}.h"
/*
 * hmap_create: construct a new hash table.
 * Number of buckets is larger of input value and pre-defined value
 */
//START_FUNC_DECL
hmap_${qkeytype}_t *
${fn}(
      size_t minsize,
      size_t sizeof_val
        )
{
  hmap_${qkeytype}_t *hmap = NULL;
  hmap_bucket_${qkeytype}_t *buckets = NULL;
  void *vals = NULL;
  // following is what we support today. Can be extended
  if ( ( sizeof_val != 4 ) && ( sizeof_val != 8 ) ) { WHEREAMI; return NULL; }

  hmap = calloc(1, sizeof(hmap_${qkeytype}_t));
  if ( hmap == NULL ) { return NULL; }

  hmap->size = hmap->minsize = MAX(minsize, HASH_INIT_SIZE);

  buckets = calloc(hmap->size, sizeof(hmap_bucket_${qkeytype}_t));
  if ( buckets == NULL ) { free(hmap); return NULL; }
  hmap->buckets = (void *)buckets;

  vals = calloc(hmap->size, sizeof_val);
  if ( vals == NULL ) { free(hmap); free(buckets); return NULL; }
  hmap->vals = (void *)vals;

  return hmap;
}
]]
}

