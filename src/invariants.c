/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "q_rhashmap.h"
#include "invariants.h"
static int
compar_fn(
      const void *p1, 
      const void *p2
      )
{
  KEYTYPE k1 = *((const KEYTYPE *)p1);
  KEYTYPE k2 = *((const KEYTYPE *)p2);
  if ( k1 < k2 ) { 
    return 1;
  }
  else {
    return 0;
  }
}

//---------------------------------------
int
invariants(
    q_rhashmap_t *hmap
    )
{
  int status = 0;
  KEYTYPE *keys = NULL; uint32_t nkeys = 0;
  if ( hmap == NULL ) { return 0; }

  if ( hmap->hashkey == 0 ) { go_BYE(-1); }
  if ( hmap->nitems >= hmap->size ) { go_BYE(-1); }
  if ( hmap->divinfo == 0 ) { go_BYE(-1); }
  if ( hmap->buckets == NULL ) { go_BYE(-1); }
  if ( hmap->minsize == 0 ) { go_BYE(-1); }
  if ( hmap->size == 0 ) { go_BYE(-1); }
  if ( hmap->size < hmap->minsize ) { go_BYE(-1); }
  q_rh_bucket_t *buckets = hmap->buckets;
  // Allocate array "keys" of size nitems
  keys = malloc(hmap->nitems * sizeof(KEYTYPE));
  return_if_malloc_failed(keys);
  for ( unsigned int i = 0; i < hmap->size; i++ ) { 
    if ( buckets[i].key == 0 ) { 
       if ( buckets[i].hash != 0 ) { go_BYE(-1); }
      // Following is a defensive procedure i.e., zero out null entries
      if ( buckets[i].val  != 0 ) { go_BYE(-1); }
      if ( buckets[i].psl  != 0 ) { go_BYE(-1); }
    }
    else {
      if ( buckets[i].hash == 0 ) { go_BYE(-1); }
      // Accumulate all keys into array called "keys"
      keys[nkeys++] = buckets[i].key;
    }
  }
  qsort(keys, nkeys, sizeof(KEYTYPE), compar_fn);
  // Sort array "keys"
  for ( unsigned int i = 1; i  < nkeys; i++ ) { 
    if ( keys[i] == keys[i-1] ) { go_BYE(-1); }
  }
  if ( hmap->nitems != nkeys ) { go_BYE(-1); }
BYE:
  free_if_non_null(keys);
  return status;
}
