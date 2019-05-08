/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "q_rhashmap.h"

static void
test_basic(void)
{
  q_rhashmap_t *hmap;
  __VALTYPE__ ret;

  hmap = q_rhashmap_create(0, 0);
  assert(hmap != NULL);

  ret = q_rhashmap_get(hmap, 123);
  assert(ret == 0);

  ret = q_rhashmap_put(hmap, 123, 456);
  assert(ret == 456);

  ret = q_rhashmap_get(hmap, 123);
  assert(ret == 456);

  ret = q_rhashmap_del(hmap, 123);
  assert(ret == 456);

  ret = q_rhashmap_get(hmap, 123);
  assert(ret == 0);
  if ( ret != 0 ) { exit(1); }

  q_rhashmap_destroy(hmap);
}

int
main(void)
{
  test_basic();
  puts("ok");
  return 0;
}
