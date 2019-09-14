/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */
//START_INCLUDES
#include "hmap_common.h"
#include "_hmap_types.h"
//STOP_INCLUDES
#include "_hmap_chk.h"
//START_FUNC_DECL
int 
hmap_chk(
    hmap_t *ptr_hmap
    )
//STOP_FUNC_DECL
{
  int status = 0;
  if ( ptr_hmap->bkts == 0 ) { 
    if ( ptr_hmap->nitems != 0 ) { go_BYE(-1); }
    if ( ptr_hmap->minsize != 0 ) { go_BYE(-1); }
    if ( ptr_hmap->divinfo != 0 ) { go_BYE(-1); }
    if ( ptr_hmap->hashkey != 0 ) { go_BYE(-1); }
    if ( ptr_hmap->size != 0 ) { go_BYE(-1); }
    return status;
  }

  if ( ptr_hmap == NULL ) { return status; }
  if ( ptr_hmap->nitems > ptr_hmap->size ) { go_BYE(-1); }
  if ( ptr_hmap->size < ptr_hmap->minsize ) { go_BYE(-1); }
  if ( ptr_hmap->divinfo == 0 ) { go_BYE(-1); }
  if ( ptr_hmap->hashkey == 0 ) { go_BYE(-1); }
  if ( ptr_hmap->bkts == NULL ) { go_BYE(-1); }
BYE:
  return status;
}
