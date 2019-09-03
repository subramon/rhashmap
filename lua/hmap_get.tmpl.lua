return require 'Q/UTILS/lua/code_gen' {
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"
extern int ${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key,
    ${caggvaltype} *ptr_val,
    bool *ptr_is_found
    );
    ]],
definition = [[
#include "_${fn}.h"
extern int ${fn}(
    hmap_t *ptr_hmap, 
    ${ckeytype}  key,
    ${caggvaltype} *ptr_val,
    bool *ptr_is_found
)
{
/*
 * hmap_get: lookup an value given the key.
 * => If key is present, *ptr_val is set to its associated value
 * and is_found is set to true
 * => If key is absent, *ptr_val is set to 0
 * and is_found is set to false
 */
  int status = 0;
  const uint32_t hash = murmurhash3(
    &key, sizeof(${ckeytype}), ptr_hmap->hashkey);
  uint32_t n = 0; 
  // loc is location where we look for key 
  uint32_t loc = fast_rem32(hash, ptr_hmap->size, ptr_hmap->divinfo);
  *ptr_is_found = false;
  memset(ptr_val, '\0', sizeof(${caggvaltype}));

  ${ckeytype}    *keys = ptr_hmap->keys;
  ${caggvaltype} *vals = ptr_hmap->vals;
  uint16_t       *psls = ptr_hmap->psls;
#ifdef DEBUG
  uint32_t     *hashes = ptr_hmap->hashes;
#endif

  /*
   * Lookup is a linear probe.
   */
  register uint64_t divinfo = ptr_hmap->divinfo;
  register uint64_t size    = ptr_hmap->size;
  for ( ; ; ) { 
#ifdef DEBUG
    ASSERT(validate_psl_p(hmap, i));
    if ( ( hashes[loc] == hash ) && ( keys[loc] == key ) )
#else
    if ( keys[loc] == key ) 
#endif
    {
      *ptr_val = vals[loc];
      *ptr_is_found = true;
      break;
    }
    /*
     * Stop probing if we hit an empty bucket; also, if we hit a
     * bucket with PSL lower than the distance from the base location,
     * then it means that we found the "rich" bucket which should
     * have been captured, if the key was inserted -- see the central
     * point of the algorithm in the insertion function.
     */
    if ( ( keys[loc] == 0 ) || ( n > psls[loc] ) ) {
      *ptr_is_found = false;
      break;
    }
    n++;
    /* Continue to the next bucket. */
    loc = fast_rem32(loc + 1, size, divinfo);
  }
  return status;
}
]],
}
