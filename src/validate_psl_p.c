// TODO Need to incorporate this check into the code
//START_INCLUDES
#include "hmap_common.h"
#include "_hmap_types.h"
//STOP_INCLUDES
#include "_validate_psl_p.h"
//START_FUNC_DECL
int __attribute__((__unused__))
validate_psl_p(
    hmap_t *hmap, 
    uint32_t i
    )
//STOP_FUNC_DECL
{
#ifdef DEBUG
  uint32_t base_i = fast_rem32(hmap->hashes[i], hmap->size, hmap->divinfo);
  uint32_t diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
  return hmap->keys[i] == 0 || diff == hmap->psls[i];
#else
  return 0;
#endif
}
