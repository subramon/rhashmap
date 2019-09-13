/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

// gcc -g  $QC_FLAGS test2.c ../lua/libtest2.so -I../xgen_inc/ -I../inc/
// TODO: Write tests on cnt 
#include "_hmap_create.h"
#include "_hmap_destroy.h"
#include "_hmap_put.h"
#include "_hmap_get.h"
#include "_hmap_del.h"
#include "_hmap_chk_no_holes.h"

static int
test_basic(
    void
    )
{
  int status = 0;
  hmap_t *ptr_hmap = NULL;
  uint32_t N = 1000000;

  ptr_hmap = hmap_create(0);
  if ( ptr_hmap == NULL) { go_BYE(-1); }
  uint64_t num_probes = 0;
  val_t inval, altval, oldval;
  fprintf(stderr, "Put keys for first time, value = key \n");
  keytype key;
  cnttype cnt;
  for ( uint32_t i = 0; i < N; i++ ) { 
    bool is_updated, is_found;
    key = i+1;
    cnt = 0;
    inval.val_1 = i+1;
    status = hmap_put(ptr_hmap, key, inval, &oldval, &is_updated, &num_probes);
    cBYE(status);
    if ( ptr_hmap->nitems != i+1 ) { go_BYE(-1); }
    if ( is_updated == true ) { go_BYE(-1); }
    if ( cnt  != 0  ) { go_BYE(-1); }
    if ( oldval.val_1 != 0  ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &cnt, &is_found, &num_probes);
    cBYE(status);
    if ( altval.val_1 != inval.val_1 ) { 
      printf("hello world\n");
      go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
  }
  fprintf(stderr, "Put keys for second time, value = key \n");
  for ( uint32_t i = 0; i < N; i++ ) { 
    bool is_updated, is_found;
    key = i+1;
    inval.val_1 = i+1;
    status = hmap_put(ptr_hmap, key, inval, &oldval, &is_updated, &num_probes);
    cBYE(status);
    if ( ptr_hmap->nitems != N ) { go_BYE(-1); }
    if ( is_updated == false ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &cnt, &is_found, &num_probes);
    cBYE(status);
    if ( altval.val_1 != 2*inval.val_1 ) { go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
  }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
  fprintf(stderr, "Delete keys \n");
  for ( uint32_t i = 0; i < N; i++ ) { 
    bool is_found;
    key = i+1;
    status = hmap_del(ptr_hmap, key, &is_found, &oldval, &num_probes);
    cBYE(status);
    if ( is_found == false ) { go_BYE(-1); }
    if ( ptr_hmap->nitems != N-(i+1) ) { go_BYE(-1); }
    if ( oldval.val_1 != 2*(i+1) ) { go_BYE(-1); }
    if ( ( i % 1000 ) == 0 ) { 
      status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
    }
    status = hmap_get(ptr_hmap, key, &altval, &cnt, &is_found, &num_probes);
    cBYE(status);
    if ( is_found ) { go_BYE(-1); }
  }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
  fprintf(stderr, " Delete keys again \n");
  for ( uint32_t i = 0; i < N; i++ ) { 
    bool is_found;
    key = i+1;
    status = hmap_del(ptr_hmap, key, &is_found, &oldval, &num_probes);
    cBYE(status);
    if ( is_found == true ) { go_BYE(-1); }
    if ( ptr_hmap->nitems != 0 ) { go_BYE(-1); }
  }
  hmap_destroy(ptr_hmap); 
BYE:
  return status;
}

int main(void)
{
  int status = 0;
  status = test_basic(); cBYE(status);
  printf("Success\n");
BYE:
  return status;
}
