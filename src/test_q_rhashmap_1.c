/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "q_rhashmap.h"
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
//---------------------------------------

static int
test_basic(
    void
    )
{
  //C \section{Basic test}
  //C Simplest test.
  int status = 0;
  q_rhashmap_t *hmap = NULL;
  VALTYPE val = 0, oldval;
  bool key_exists;
  KEYTYPE key = 123;
  uint64_t t_stop, t_start = RDTSC();
  bool is_found;

  //C \begin{itemize}
  //C \item Create a hashmap with default size 0. 
  hmap = q_rhashmap_create(0);
  //C Should succeed.
  if ( hmap == NULL) { go_BYE(-1); }

  //C \item Look for a key that has not been inserted. 
  status = q_rhashmap_get(hmap, key, &val, &is_found); cBYE(status);
  //C Should not be found. Returned value should be 0.
  if ( is_found ) { go_BYE(-1); }
  if ( val != 0 ) { go_BYE(-1); }

  //C \item Put the same key {\tt iters} times, starting with value 0 
  //C and incrementing value by 1 each time through the loop.
  int iters = 10;
  for ( int i = 0; i < 10; i++ ) { 
    VALTYPE chk_oldval = val;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_SET, &oldval);
    cBYE(status);
    if ( oldval != chk_oldval ) { go_BYE(-1); }
  }
  //C Each time, old value of key should be previous value.
  //C \item Get value of key at end of loop.
  status = q_rhashmap_get(hmap, key, &val, &is_found); cBYE(status);
  //C Should be iters
  if ( !is_found ) { go_BYE(-1); }
  if ( val != iters ) { go_BYE(-1); }

  //C \item Delete the key.
  status = q_rhashmap_del(hmap, key, &oldval, &key_exists); cBYE(status);
  //C Delete should succeed and indicate key existed.
  if ( ! key_exists ) { go_BYE(-1); }

  //C \item Delete the key again.
  status = q_rhashmap_del(hmap, key, &oldval, &key_exists); cBYE(status);
  //C Delete should succeed and indicate key did not exist.
  if ( key_exists ) { go_BYE(-1); }

  //C \item Get the value of the key.
  status = q_rhashmap_get(hmap, key, &val, &is_found); cBYE(status);
  //C Should not be found
  if ( is_found ) { go_BYE(-1); }
  if ( val != 0 ) { go_BYE(-1); }

  //C \item Destroy the hashmap
  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  //C \end{itemize}
  fprintf(stderr, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  return status;
}
int main(void)
{
  int status = 0;
  status = test_basic();       cBYE(status);
BYE:
  return status;
}
