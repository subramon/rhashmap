/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */
//START_INCLUDES
#include "hmap_common.h"
//STOPINCLUDES
#include "_hmap_mk_loc.h"
//START_FUNC_DECL
int 
hmap_mk_loc(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t hmap_size, // input 
    uint32_t *locs // output [nkeys] 
    )
//STOP_FUNC_DECL
{
  int status = 0;
  uint64_t divinfo = fast_div32_init(hmap_size);
#pragma omp parallel for schedule(static, RH_CHUNK_SIZE)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    locs[i] = fast_rem32(hashes[i], hmap_size, divinfo);
  }
  return status;
}
