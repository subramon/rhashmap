return require 'Q/UTILS/lua/code_gen'   {
declaration = [[
#include "hmap_common.h"
#include "_hmap_type_${qkeytype}.h"
#include "_hmap_insert_${qkeytype}_${qvaltype}.h"
// typedef hmap_bucket_${qkeytype}_t bkt_type ;
extern int
${fn}(
    hmap_${qkeytype}_t *hmap, 
    size_t newsize,
    int *ptr_num_probes
    );
]],
definition = [[
#include "_${fn}.h"

int
${fn}(
    hmap_${qkeytype}_t *ptr_hmap, 
    size_t newsize,
    int *ptr_num_probes
    )
{
  int status = 0;
  bkt_type *oldbuckets = ptr_hmap->buckets;
  const size_t oldsize = ptr_hmap->size;
  bkt_type *newbuckets = NULL;
  ${cvaltype} *newvals    = NULL;
  ${cvaltype} *oldvals    = ptr_hmap->vals;
  int num_probes      = 0;


  // some obvious logical checks
  if ( ( oldbuckets == NULL ) && ( oldsize != 0 ) ) { go_BYE(-1); }
  if ( ( oldbuckets != NULL ) && ( oldsize == 0 ) ) { go_BYE(-1); }
  if ( ( newsize <= 0 ) || ( newsize >= UINT_MAX ) )  { go_BYE(-1); }
  if ( newsize <= ptr_hmap->nitems ) { go_BYE(-1); }

  // allocate buckets.  
  newbuckets = calloc(newsize, sizeof(bkt_type));
  return_if_malloc_failed(newbuckets);

  newvals = calloc(newsize, sizeof(${cvaltype}));
  return_if_malloc_failed(newvals);

  ptr_hmap->vals    = newvals;
  ptr_hmap->buckets = newbuckets;
  ptr_hmap->size    = newsize;
  ptr_hmap->nitems  = 0;

   // generate a new hash key/seed every time we resize the hash table.
  ptr_hmap->divinfo = fast_div32_init(newsize);
  ptr_hmap->hashkey ^= random() | (random() << 32);

  for ( uint32_t i = 0; i < oldsize; i++) {
    const bkt_type *bucket = oldbuckets + i;
    /* Skip the empty buckets. */
    if ( !bucket->key ) { continue; }
    ${cvaltype}  oldval;  // needed only to match function signature
    status = hmap_insert_${qkeytype}_${qvaltype}(
      ptr_hmap, bucket->key, oldvals[i], &oldval, &num_probes);
    cBYE(status);
  }
  free_if_non_null(oldbuckets);
  free_if_non_null(oldvals);
  *ptr_num_probes = num_probes;
BYE:
  return status;
}
]],
}
