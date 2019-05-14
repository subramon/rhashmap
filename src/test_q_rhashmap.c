/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "q_rhashmap.h"

static int
test_basic(
    void
    )
{
  int status = 0;
  q_rhashmap_t *hmap = NULL;
  VALTYPE ret, oldval;
  bool key_exists;
  KEYTYPE key = 123;

  hmap = q_rhashmap_create(0);
  assert(hmap != NULL);

  ret = q_rhashmap_get(hmap, key);
  assert(ret == 0);

  VALTYPE val = 0;
  for ( int i = 0; i < 10; i++ ) { 
    VALTYPE chk_oldval = val;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_SET, &oldval);
    cBYE(status);
    if ( oldval != chk_oldval ) { go_BYE(-1); }
  }

  status = q_rhashmap_del(hmap, key, &key_exists, &oldval); cBYE(status);
  if ( ! key_exists ) { go_BYE(-1); }
  assert(ret == val);

  ret = q_rhashmap_get(hmap, key);
  assert(ret == 0);
  if ( ret != 0 ) { exit(1); }

  q_rhashmap_destroy(hmap);
  fprintf(stderr, "Passsed  %s \n", __func__);
BYE:
  return status;
}
static int
test_add_a_lot(
    void
    )
{
  int status = 0;
  q_rhashmap_t *hmap = NULL;
  VALTYPE val, oldval;
  bool key_exists;
  KEYTYPE key;
  uint32_t  N = 1048576;

  hmap = q_rhashmap_create(0);
  assert(hmap != NULL);

  val = key = 0;
  for ( uint32_t i = 0; i < N; i++ ) { 
    status = q_rhashmap_put(hmap, ++key, ++val, Q_RHM_SET, &oldval);
    cBYE(status);
    if ( oldval != 0 ) { go_BYE(-1); }
    if ( hmap->nitems != i+1 ) { go_BYE(-1); }
  }
  if ( hmap->nitems != N ) { go_BYE(-1); }

  val = key = 0;
  for ( uint32_t i = 0; i < N; i++ ) { 
    ++val;
    status = q_rhashmap_del(hmap, ++key, &key_exists, &oldval); cBYE(status);
    if ( !key_exists ) { go_BYE(-1); }
    if ( oldval != val ) { go_BYE(-1); }
  }
  if ( hmap->nitems != 0 ) { go_BYE(-1); }
  q_rhashmap_destroy(hmap);
  fprintf(stderr, "Passsed  %s \n", __func__);
BYE:
  return status;
}
static int
test_incr(
    void
    )
{
  int status = 0;
  q_rhashmap_t *hmap = NULL;
  KEYTYPE key = 123;
  VALTYPE val = 0;
  int64_t sumval = 0;

  hmap = q_rhashmap_create(0);
  if ( hmap == NULL ) { go_BYE(-1); }

  for ( int i = 0; i < 10000; i++ ) { 
    VALTYPE oldval;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_INCR, &oldval);
    cBYE(status);
    if ( oldval != sumval ) { go_BYE(-1); }
    sumval += val;
  }
  q_rhashmap_destroy(hmap);
  fprintf(stderr, "Passsed  %s \n", __func__);
BYE:
  return status;
}

int
main(void)
{
  int status = 0;
  status = test_basic();     cBYE(status);
  status = test_add_a_lot(); cBYE(status);
  status = test_incr();      cBYE(status);
  puts("ok");
BYE:
  return status;
}
