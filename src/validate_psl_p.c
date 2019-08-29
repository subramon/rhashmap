#ifdef DEBUG
#include "_validate_psl_p.h"
//START_FUNC_DECL
int __attribute__((__unused__))
validate_psl_p(
    q_rhashmap___KV___t *hmap, 
    const q_rh_bucket___KV___t *bucket, 
    uint32_t i
    )
//STOP_FUNC_DECL
{
  uint32_t base_i = fast_rem32(bucket->hash, hmap->size, hmap->divinfo);
  uint32_t diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
  return bucket->key == 0 || diff == bucket->psl;
}
#endif
