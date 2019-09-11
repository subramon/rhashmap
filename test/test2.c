/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "_hmap_create.h"
#include "_hmap_destroy.h"
#include "_hmap_put.h"
#include "_hmap_get.h"

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
  uint32_t N = 10000000;

  ptr_hmap = hmap_create(0);
  if ( ptr_hmap == NULL) { go_BYE(-1); }
  uint64_t num_probes = 0;
  for ( uint32_t i = 0; i < N; i++ ) { 
    valtype altval, oldval;
    bool is_updated, is_found;
    keytype key = i+1;
    invaltype inval = i+1;
    if ( inval == 1834 ) { 
      printf("hello world \n");
    }
    status = hmap_put(ptr_hmap, key, &inval, &oldval, &is_updated, &num_probes);
    cBYE(status);
    if ( ptr_hmap->nitems != i+1 ) { go_BYE(-1); }
    if ( is_updated == true ) { go_BYE(-1); }
    status = hmap_get(ptr_hmap, key, &altval, &is_found, &num_probes);
    cBYE(status);
    if ( altval != inval ) { 
      printf("key = %lu, inval = %f, altval = %lf \n",
          key, inval, altval);
      go_BYE(-1); }
    if ( !is_found ) { go_BYE(-1); }
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
