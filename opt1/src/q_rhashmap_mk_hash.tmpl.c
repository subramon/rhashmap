#include "q_rhashmap_common.h"
#include "_q_rhashmap_mk_hash___K__.h"

//------------------------------------------------------
int 
q_rhashmap_mk_hash___K__(
    __KEYTYPE__ *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    )
{
  int status = 0;
  int chunk_size = 1024;
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    hashes[i] = murmurhash3(&(keys[i]), sizeof(__KEYTYPE__), hmap_hashkey);
  }
  return status;
}
