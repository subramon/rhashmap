#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "macros.h"
#include "fastdiv.h"

 /*
 * The following example computes q = a / b and r = a % b:
 *
 *	uint64_t divinfo = fast_div32_init(b);
 *	q = fast_div32(a, b, divinfo);
 *	r = fast_rem32(a, b, divinfo);
 */
static int
test_fastdiv(
    uint32_t a,
    uint32_t b
    )
{
  int status = 0;

  uint64_t divinfo = fast_div32_init(b);
  uint32_t q = fast_div32(a, b, divinfo);
  uint32_t r = fast_rem32(a, b, divinfo);
  if ( q != a / b ) { go_BYE(-1); }
  if ( r != a % b ) { go_BYE(-1); }
BYE:
  return status;
}

int
main(void)
{
  int status = 0;
  for ( int i = 0; i < 1000000; i++ ) { 
    uint32_t a = random();
    uint32_t b = random();
    status = test_fastdiv(a, b); cBYE(status);
  }
  fprintf(stdout, "SUCCESS\n");
BYE:
  return status;
}

