/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "q_rhashmap.h"
#include "invariants.h"
//---------------------------------------
static uint64_t
RDTSC(
    void
    )
{
  unsigned int lo, hi;
  asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}
//----------------------------------------------------------
static int
test_add_a_lot(
    void
    )
{
  int status = 0;
  int np = 0; // number of probes
  q_rhashmap_t *hmap = NULL;
  VALTYPE val, oldval;
  bool key_exists;
  KEYTYPE key;
  uint32_t  N = 1048576;
  uint64_t t_stop, t_start = RDTSC();
  bool is_found;

  //C \section{A lot of puts}
  //C \begin{itemize}
  //C \item Create a hashmap
  hmap = q_rhashmap_create(0);
  if (hmap == NULL) { go_BYE(-1); }

  int niters = 10;
  //C \item Perform the outer loop {\tt niters} times.
  for ( int iter = 0; iter < niters; iter++ ) { 
    uint32_t curr_size = hmap->size;
    val = key = 0;
    //C \begin{itemize}
    //C \item Perform the inner loop {\tt N} times.
    //C \begin{itemize}
    for ( uint32_t i = 0; i < N; i++ ) {
      VALTYPE test_val;
      //C \item Put a unique (key,val). 
      //C In iterations \(i\) from 1 to N, 
      //C we put key as \(i\), value as \(i\).
      status = q_rhashmap_put(hmap, ++key, ++val, Q_RHM_SET, &oldval, &np);
      cBYE(status);
      //C After each put,
      //C Verify that the number of items in the hmap increased by 1.
      if ( oldval != 0 ) { go_BYE(-1); }
      if ( hmap->nitems != i+1 ) { go_BYE(-1); }

      //C \item Get the value for the key just inserted.
      status = q_rhashmap_get(hmap, key, &test_val, &is_found); 
      cBYE(status);
      //C Verify that the value is correct.
      if ( !is_found ) { go_BYE(-1); }
      if ( test_val != val ) { go_BYE(-1); }
      //C \item Check that number of items in hmap is less than 
      //CX 90\% of size of hmap.
      if ( hmap->nitems > 0.9 * hmap->size ) { go_BYE(-1); }
      //C \item If number of items \(> 1024\), then number of items 
      // should be at least 10\% of siz of hmap
      if ( ( hmap->nitems > 1024 ) && 
            ( hmap->nitems < 0.1 * hmap->size ) ) { go_BYE(-1); }
      if ( hmap->size != curr_size ) { 
        curr_size = hmap->size;
      }
    }
    status = invariants(hmap); cBYE(status);
    //C \end{itemize}
    //C \item Verify that number of items in hmap is N
    if ( hmap->nitems != N ) { go_BYE(-1); }


    val = key = 0;
    //C \begin{itemize}
    for ( uint32_t i = 0; i < N; i++ ) {
      ++val;
      //C \item Delete each key inserted.
      status = q_rhashmap_del(hmap, ++key, &oldval, &key_exists); 
      cBYE(status);
      if ( !key_exists ) { go_BYE(-1); }
      if ( oldval != val ) { go_BYE(-1); }
      //C Delete should indicate that key did exist and its value 
      //C was what was inserted in previous loop
      if ( hmap->nitems > 0.9 * hmap->size ) { go_BYE(-1); }
      if ( ( hmap->nitems > 1024 ) && 
            ( hmap->nitems < 0.1 * hmap->size ) ) { go_BYE(-1); }
      if ( hmap->size != curr_size ) { 
        curr_size = hmap->size;
      }
    }
    status = invariants(hmap); cBYE(status);
    //C \end{itemize}
    //C \item Verify that number of items in hmap is 0.
    if ( hmap->nitems != 0 ) { go_BYE(-1); }
    //C \end{itemize}
  }
  //C \item Destroy the hmap
  status = invariants(hmap); cBYE(status);
  q_rhashmap_destroy(hmap);
  //C \end{itemize}
  t_stop = RDTSC();
  fprintf(stdout, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  return status;
}
//----------------------------------------------------------
int main(void) {
  int status = 0;
  status = test_add_a_lot();   cBYE(status);
BYE:
  return status;
}
