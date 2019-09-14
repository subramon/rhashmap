return  require 'Q/UTILS/lua/code_gen' { 
declaration = [[ 
#include "hmap_common.h"
extern int 
${fn}(
    ${ckeytype} *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    );
    ]],
definition = [[
#include "_${fn}.h"
int 
${fn}(
    ${ckeytype} *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    )
{
  int status = 0;
  int chunk_size = 1024;
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    hashes[i] = murmurhash3(&(keys[i]), sizeof(${ckeytype}), hmap_hashkey);
  }
  return status;
}
]],
}

