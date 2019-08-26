#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "macros.h"
#include "fastdiv.h"

static uint64_t
RDTSC(
    void
    )
{
  unsigned int lo, hi;
  asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

int
main(void)
{
  int status = 0;
  int N = 16 * 1048576;
  uint32_t *in = NULL;
  uint32_t *out = NULL;
  in  = malloc(N * sizeof(uint32_t));
  out = malloc(N * sizeof(uint32_t));
  uint32_t b = random();
  uint64_t divinfo = fast_div32_init(b);
  for ( int i = 0; i < N; i++ ) { 
    in[i] = random();
  }
  uint64_t t0 = RDTSC();
  for ( int i = 0; i < N; i++ ) { 
    out[i] = fast_rem32(in[i], b, divinfo); 
  }
  uint64_t t1 = RDTSC();
  for ( int i = 0; i < N; i++ ) { 
    out[i] = in[i] % b;
  }
  uint64_t t2 = RDTSC();
  fprintf(stdout, "fastdiv = %" PRIu64 "\n", (t1 - t0));
  fprintf(stdout, "regular = %" PRIu64 "\n", (t2 - t1));
BYE:
  return status;
}

/* some results below show that fast_32 is faster but not by a lot
 *
test:(master)$ gcc -O4 -std=gnu99 -I../src/ perf_fastdiv.c
test:(master)$ ./a.out
fastdiv = 70289146
regular = 79893842
test:(master)$ ./a.out
fastdiv = 71935338
regular = 77695654
test:(master)$ ./a.out
fastdiv = 72003454
regular = 77096528
test:(master)$ ./a.out
fastdiv = 69566454
regular = 77636310
test:(master)$ ./a.out
fastdiv = 70777246
regular = 78877218
test:(master)$ ./a.out
fastdiv = 75635344
regular = 80307570
*/
