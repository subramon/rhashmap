/*
 * rhashmap_destroy: free the memory used by the hash table.
 */
//START_INCLUDES
#include "hmap_common.h"
//STOP_INCLUDES
//START_FUNC_DECL
void
q_rhashmap_destroy(
    q_rhashmap_t *ptr_hmap
    )
//STOP_FUNC_DECL
{
  free(ptr_hmap->buckets);
  ptr_hmap->buckets = NULL;
  ptr_hmap->size = 0;
  ptr_hmap->nitems = 0;
  ptr_hmap->divinfo = 0;
  ptr_hmap->buckets = 0;
  ptr_hmap->hashkey = 0;
  ptr_hmap->minsize = 0;

  free(ptr_hmap);
  ptr_hmap = NULL;
}
