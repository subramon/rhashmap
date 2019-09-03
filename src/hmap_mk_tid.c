/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

//------------------------------------------------------
//START_INCLUDES
#include "hmap_common.h"
//STOP_INCLUDES
#include "_hmap_mk_tid.h"

/* Ideally, we want to distribute the work to the threads so that
 * 1) they never update the same cell
 * 2) they (ideally) have large contiguous regions which they own i.e., 
 * only they write in that region

Dividing based on hashes gives us 1)
Dividing based on locs   gives us 2)
However, since 1) is more important than 2), we went with 1)
Note that locs doesn't give you the location of a key.
It only gives you a starting point for the hunt for the location of a key
 */

//START_FUNC_DECL
int 
hmap_mk_tid(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t nT, // input , number of threads
    uint8_t *tids // output [nkeys] 
    )
//STOP_FUNC_DECL
{
  int status = 0;
  int chunk_size = 1024;
  uint64_t divinfo = fast_div32_init(nT);
#pragma omp parallel for schedule(static, chunk_size)
  for ( uint32_t i = 0; i < nkeys; i++ ) {
    tids[i] = fast_rem32(hashes[i], nT, divinfo);
  }
  return status;
}
