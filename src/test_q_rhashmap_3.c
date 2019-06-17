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
test_incr(
    void
    )
{
  //C \section{Test increment}
  //C \label{test_incr}
  //C \begin{itemize}
  int status = 0;
  int np = 0; // number of probes
  q_rhashmap_t *hmap = NULL;
  KEYTYPE key = 123;
  VALTYPE val = 0;
  int64_t sumval = 0;
  uint64_t t_stop, t_start = RDTSC();

  hmap = q_rhashmap_create(0);
  if ( hmap == NULL ) { go_BYE(-1); }

  //C \item Create a hashmap
  //C \item Set the update type to ADD
  //C \item For the same key, perform N puts. In the first put,
  //C set value to 1, in the second to 2
  //C \item At the end, the value for that key should be \(N \times (N+1)/2\)
  for ( int i = 0; i < 10000; i++ ) { 
    VALTYPE oldval;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_ADD, &oldval, &np);
    cBYE(status);
    if ( oldval != sumval ) { go_BYE(-1); }
    sumval += val;
    if ( hmap->nitems != 1 ) { go_BYE(-1); }
  }
  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  fprintf(stdout, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
  //C \end{itemize}
BYE:
  return status;
}
//----------------------------------------------------------
int main(void)
{
  int status = 0;
  status = test_incr();        cBYE(status);
BYE:
  return status;
}
