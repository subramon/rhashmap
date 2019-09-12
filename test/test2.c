/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

// gcc -g  $QC_FLAGS test2.c ../lua/libfoobar.so -I../xgen_inc/ -I../inc/
#include "_hmap_create.h"
#include "_hmap_destroy.h"
#include "_hmap_put.h"
#include "_hmap_get.h"
#include "_hmap_del.h"
#include "_hmap_chk_no_holes.h"

typedef uint64_t keytype ;
typedef float invaltype ;
typedef double valtype ;
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
  for ( uint32_t i = 0; i < N; i++ ) { 
    valtype altval, oldval;
    bool is_updated, is_found;
    keytype key = i+1;
    invaltype inval = i+1;
    status = hmap_put(ptr_hmap, key, &inval, &oldval, &is_updated, &num_probes);
    cBYE(status);
    if ( ptr_hmap->nitems != i+1 ) { go_BYE(-1); }
    if ( is_updated == true ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &is_found, &num_probes);
    cBYE(status);
    if ( altval != inval ) { go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
  }
  // Put keys (with same value as before) in one more time 
  for ( uint32_t i = 0; i < N; i++ ) { 
    valtype altval, oldval;
    bool is_updated, is_found;
    keytype key = i+1;
    invaltype inval = i+1;
    status = hmap_put(ptr_hmap, key, &inval, &oldval, &is_updated, &num_probes);
    cBYE(status);
    if ( ptr_hmap->nitems != N ) { go_BYE(-1); }
    if ( is_updated == false ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &is_found, &num_probes);
    cBYE(status);
    if ( altval != 2*inval ) { go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
  }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
  fprintf(stderr, " Delete keys \n");
  for ( uint32_t i = 0; i < N; i++ ) { 
    valtype oldval, altval;
    bool is_found;
    keytype key = i+1;
    status = hmap_del(ptr_hmap, key, &is_found, &oldval, &num_probes);
    cBYE(status);
    if ( oldval != 2*(i+1) ) { go_BYE(-1); }
    if ( ( i % 1000 ) == 0 ) { 
      status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
    }
    if ( is_found == false ) { go_BYE(-1); }
    if ( ptr_hmap->nitems != N-(i+1) ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &is_found, &num_probes);
    cBYE(status);
    if ( is_found ) { go_BYE(-1); }
  }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
  fprintf(stderr, " Delete keys again \n");
  for ( uint32_t i = 0; i < N; i++ ) { 
    valtype oldval;
    bool is_found;
    keytype key = i+1;
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
