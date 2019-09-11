/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "_hmap_create.h"
#include "_hmap_destroy.h"
#include "_hmap_put.h"

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
  int N = 10;

  ptr_hmap = hmap_create(0);
  if ( ptr_hmap == NULL) { go_BYE(-1); }
  keytype key = 1;
  uint64_t num_probes = 0;
  valtype chk_oldval = 0;
  for ( int i = 0; i < N; i++ ) { 
    bool is_updated;
    invaltype inval = i + 1;
    valtype oldval;
    status = hmap_put(ptr_hmap, key, &inval, &oldval, &is_updated, &num_probes);
    if ( num_probes != 0 ) { go_BYE(-1); }
    if ( ptr_hmap->nitems != 1 ) { go_BYE(-1); }
    if ( i == 0 ) { 
      if ( is_updated == true ) { go_BYE(-1); }
    }
    else {
      if ( is_updated == false ) { go_BYE(-1); }
      if ( oldval != chk_oldval ) { 
        printf("i = %d \n", i);
        go_BYE(-1); }
    }
    cBYE(status);
    chk_oldval += inval;

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
