//START_INCLUDES
#include "hmap_common.h"
#include "_hmap_types.h"
//STOP_INCLUDES
#include "_hmap_destroy.h"
//START_FUNC_DECL
void
hmap_destroy(
    hmap_t *ptr_hmap
    )
//STOP_FUNC_DECL
{
  if ( ptr_hmap == NULL ) { return; }
#ifdef DEBUG
  free_if_non_null(ptr_hmap->hashes);
#endif
  free_if_non_null(ptr_hmap->vals);
  free_if_non_null(ptr_hmap->keys);
  free_if_non_null(ptr_hmap->psls);
  ptr_hmap->size = 0;
  ptr_hmap->nitems = 0;
  ptr_hmap->divinfo = 0;
  ptr_hmap->hashkey = 0;
  ptr_hmap->minsize = 0;
  free_if_non_null(ptr_hmap);
}
