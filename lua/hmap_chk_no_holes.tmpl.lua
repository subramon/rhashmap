return require 'Q/UTILS/lua/code_gen' { 
declaration = [[
#include "hmap_common.h"
#include "_hmap_types.h"

typedef struct _chk_rec_type { 
  ${ckeytype} key;
  uint32_t loc;
  uint32_t ploc;
  uint32_t hsh;
} CHK_REC_TYPE;

extern int
${fn}(
    hmap_t *ptr_hmap
    );
]],
definition = [[
#include "_${fn}.h"

static int 
sortfn( const void *in_x, const void *in_y)
{
  CHK_REC_TYPE *x = (CHK_REC_TYPE *)in_x;
  CHK_REC_TYPE *y = (CHK_REC_TYPE *)in_y;
  if ( x->loc < y->loc ) { 
    return -1;
  }
  else if ( x->loc > y->loc ) { 
    return 1;
  }
  else {
   return 0;
  }
}

int
${fn}(
    hmap_t *ptr_hmap 
    )
{
  int status = 0;
  CHK_REC_TYPE *chks = NULL;
  if ( ptr_hmap == NULL ) { go_BYE(-1); }
  int n = ptr_hmap->nitems;
  if ( n <= 1 ) { return status; } // passes trivially

  chks = malloc(n * sizeof(CHK_REC_TYPE));
  return_if_malloc_failed(chks);
  int ctr = 0;
  for ( unsigned int i = 0; i < ptr_hmap->size; i++ ) {
    if ( ptr_hmap->bkts[i].key == 0 ) { continue; }
    if ( ctr >= n ) { go_BYE(-1); }
    chks[ctr].key = ptr_hmap->bkts[i].key;
    chks[ctr].hsh = murmurhash3(&(ptr_hmap->bkts[i].key), sizeof(${ckeytype}), 
        ptr_hmap->hashkey);
    chks[ctr].ploc = fast_rem32(chks[ctr].hsh, ptr_hmap->size, ptr_hmap->divinfo);
    chks[ctr].loc = i;
    ctr++;
  }
  if ( n != ctr ) { go_BYE(-1); }
  qsort(chks, n, sizeof(CHK_REC_TYPE), sortfn);
  for ( int i = 1; i < n; i++ ) { 
    uint32_t ploc = chks[i].ploc;
    uint32_t loc  = chks[i].loc;
    if ( loc == ploc ) { continue; }
    if ( ploc < loc ) {
      for ( unsigned int j = ploc; j <= loc; j++ ) {
        if ( ptr_hmap->bkts[j].key == 0 ) { go_BYE(-1); }
      }
    }
    else {
      for ( unsigned int j = ploc; j < ptr_hmap->size; j++ ) {
        if ( ptr_hmap->bkts[j].key == 0 ) { go_BYE(-1); }
      }
      for ( unsigned int j = 0; j < loc; j++ ) { 
        if ( ptr_hmap->bkts[j].key == 0 ) { go_BYE(-1); }
      }
    }
  }
BYE:
free_if_non_null(chks);
  return status;
}
]],
}

