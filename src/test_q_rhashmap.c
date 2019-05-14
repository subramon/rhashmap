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
  int status = 0;
  q_rhashmap_t *hmap = NULL;
  VALTYPE val = 0, oldval;
  bool key_exists;
  KEYTYPE key = 123;
  uint64_t t_stop, t_start = RDTSC();
  bool is_found;

  hmap = q_rhashmap_create(0);
  assert(hmap != NULL);

  status = q_rhashmap_get(hmap, key, &val, &is_found); cBYE(status);
  if ( is_found ) { go_BYE(-1); }
  if ( val != 0 ) { go_BYE(-1); }

  for ( int i = 0; i < 10; i++ ) { 
    VALTYPE chk_oldval = val;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_SET, &oldval);
    cBYE(status);
    if ( oldval != chk_oldval ) { go_BYE(-1); }
  }

  status = q_rhashmap_del(hmap, key, &key_exists, &oldval); cBYE(status);
  if ( ! key_exists ) { go_BYE(-1); }

  status = q_rhashmap_get(hmap, key, &val, &is_found); cBYE(status);
  if ( is_found ) { go_BYE(-1); }
  if ( val != 0 ) { go_BYE(-1); }

  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  fprintf(stderr, "Passsed  %s %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  return status;
}
//----------------------------------------------------------
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
  uint64_t t_stop, t_start = RDTSC();
  bool is_found;

  hmap = q_rhashmap_create(0);
  assert(hmap != NULL);

  for ( int iter = 0; iter < 2; iter++ ) { 
    uint32_t curr_size = hmap->size;
    val = key = 0;
    for ( uint32_t i = 0; i < N; i++ ) {
      VALTYPE test_val;
      status = q_rhashmap_put(hmap, ++key, ++val, Q_RHM_SET, &oldval);
      cBYE(status);
      if ( oldval != 0 ) { go_BYE(-1); }
      if ( hmap->nitems != i+1 ) { go_BYE(-1); }

      status = q_rhashmap_get(hmap, key, &test_val, &is_found); cBYE(status);
      if ( !is_found ) { go_BYE(-1); }
      if ( test_val != val ) { go_BYE(-1); }
      //--- START: test occupancy ratio
      if ( hmap->nitems > 0.9 * hmap->size ) { go_BYE(-1); }
      if ( ( hmap->nitems > 1024 ) && 
            ( hmap->nitems < 0.1 * hmap->size ) ) { go_BYE(-1); }
      if ( hmap->size != curr_size ) { 
        curr_size = hmap->size;
      }
      //--- STOP: test occupancy ratio
    }
    if ( hmap->nitems != N ) { go_BYE(-1); }


    val = key = 0;
    for ( uint32_t i = 0; i < N; i++ ) { 
      ++val;
      status = q_rhashmap_del(hmap, ++key, &key_exists, &oldval); cBYE(status);
      if ( !key_exists ) { go_BYE(-1); }
      if ( oldval != val ) { go_BYE(-1); }
      //--- START: test occupancy ratio
      if ( hmap->nitems > 0.9 * hmap->size ) { go_BYE(-1); }
      if ( ( hmap->nitems > 1024 ) && 
            ( hmap->nitems < 0.1 * hmap->size ) ) { go_BYE(-1); }
      if ( hmap->size != curr_size ) { 
        curr_size = hmap->size;
      }
      //--- STOP: test occupancy ratio
    }
    if ( hmap->nitems != 0 ) { go_BYE(-1); }
  }
  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  fprintf(stderr, "Passsed  %s %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  return status;
}
//----------------------------------------------------------
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
  uint64_t t_stop, t_start = RDTSC();

  hmap = q_rhashmap_create(0);
  if ( hmap == NULL ) { go_BYE(-1); }

  for ( int i = 0; i < 10000; i++ ) { 
    VALTYPE oldval;
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_INCR, &oldval);
    cBYE(status);
    if ( oldval != sumval ) { go_BYE(-1); }
    sumval += val;
    if ( hmap->nitems != 1 ) { go_BYE(-1); }
  }
  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  fprintf(stderr, "Passsed  %s %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  return status;
}
//----------------------------------------------------------
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
