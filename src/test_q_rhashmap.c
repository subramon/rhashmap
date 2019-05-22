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

  status = q_rhashmap_del(hmap, key, &oldval, &key_exists); cBYE(status);
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
      status = q_rhashmap_del(hmap, ++key, &oldval, &key_exists); 
      cBYE(status);
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
    status = q_rhashmap_put(hmap, key, ++val, Q_RHM_ADD, &oldval);
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
static int
test_getn_simple(
    void
    )
{
  int status = 0;
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  uint32_t *locs = NULL;
  int nkeys = 1234567;
  q_rhashmap_t *hmap = NULL;
  VALTYPE val = 456, oldval;
  KEYTYPE key = 123;
  uint64_t t_stop, t_start = RDTSC();


  keys = calloc(nkeys, sizeof(KEYTYPE));
  return_if_malloc_failed(keys);
  vals = calloc(nkeys, sizeof(VALTYPE));
  return_if_malloc_failed(vals);
  hashes = calloc(nkeys,  sizeof(uint32_t));
  return_if_malloc_failed(hashes);
  locs = calloc(nkeys, sizeof(uint32_t));
  return_if_malloc_failed(locs);

  // create hmap and put 1 key in it 
  hmap = q_rhashmap_create(0);
  assert(hmap != NULL);

  status = q_rhashmap_put(hmap, key, val, Q_RHM_SET, &oldval);
  cBYE(status);
  for ( int i = 0; i < nkeys; i++ ) {
    if ( ( i % 2 ) == 0 ) {
      keys[i] = key;
    }
    else {
      keys[i] = -1 * key;
    }
  }

  status = q_rhashmap_murmurhash(keys, nkeys, hmap->hashkey, hashes);
  status = q_rhashmap_get_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);

  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  for ( int i = 0; i < nkeys; i++ ) {
    if ( ( i % 2 ) == 0 ) { 
      if ( vals[i] != val ) { go_BYE(-1); }
    }
    else {
      if ( vals[i] == val ) { go_BYE(-1); }
    }
  }

  q_rhashmap_destroy(hmap);
  t_stop = RDTSC();
  fprintf(stderr, "Passsed  %s %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  free_if_non_null(keys);
  free_if_non_null(vals);
  free_if_non_null(hashes);
  free_if_non_null(locs);
  return status;
}
//----------------------------------------------------------
static int
test_setn(
    void
    )
{
  int status = 0;
  KEYTYPE *keys = NULL;
  VALTYPE *vals = NULL;
  uint32_t *hashes = NULL;
  int nkeys         = 1048576;
  int n_unique_keys = 1024;
  q_rhashmap_t *hmap = NULL;
  VALTYPE oldval;
  // KEYTYPE key = 123;
  uint64_t t_stop, t_start = RDTSC();
  int update_type = Q_RHM_SET;
  uint8_t *is_founds = NULL;
  uint32_t *locs = NULL;


  locs = calloc(nkeys, sizeof(uint32_t));
  return_if_malloc_failed(locs);
  keys = calloc(nkeys, sizeof(KEYTYPE));
  return_if_malloc_failed(keys);
  vals = calloc(nkeys, sizeof(VALTYPE));
  return_if_malloc_failed(vals);
  hashes = calloc(nkeys,  sizeof(uint32_t));
  return_if_malloc_failed(hashes);
  is_founds = calloc(nkeys,  sizeof(uint8_t));
  return_if_malloc_failed(is_founds);

  // create hmap 
  hmap = q_rhashmap_create(0); if ( hmap == NULL ) { go_BYE(-1); }

  // put random keys into it, all of whom have value 1
  for ( int i = 0; i < nkeys; i++ ) {
    keys[i] = ( random() % n_unique_keys  ) + 1;
    vals[i] = 1;
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval);
    cBYE(status);
  }
  // now update value of all keys to 2 
  for ( int i = 0; i < nkeys; i++ ) { vals[i] = 2; }
  status = q_rhashmap_murmurhash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  status = q_rhashmap_setn(hmap, update_type, keys, hashes, vals, nkeys, 
      is_founds);
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( !is_founds[i] ) { go_BYE(-1); }
  }
  // now get all keys that were put in and ascertain value = 2
  status = q_rhashmap_get_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  cBYE(status);
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( vals[i] != 2 ) { go_BYE(-1); }
  }
  //--- destroy hmap
  q_rhashmap_destroy(hmap);
  hmap = q_rhashmap_create(0); if ( hmap == NULL ) { go_BYE(-1); }
  // put keys 1, 2, 3, .. into it all with value 1 
  for ( int i = 0; i < nkeys; i++ ) {
    keys[i] = i+1;
    vals[i] = 1;
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval);
    cBYE(status);
  }
  // now do an increment with value = 2 
  for ( int i = 0; i < nkeys; i++ ) { vals[i] = 2; }
  status = q_rhashmap_murmurhash(keys, nkeys, hmap->hashkey, hashes);
  cBYE(status);
  update_type = Q_RHM_ADD;
  status = q_rhashmap_setn(hmap, update_type, keys, hashes, vals, nkeys, 
      is_founds);
  // now get all keys that were put in and ascertain value = 1+2
  status = q_rhashmap_get_loc(hashes, nkeys, hmap->size, hmap->divinfo, locs);
  status = q_rhashmap_getn(hmap, keys, hashes, locs, vals, nkeys);
  cBYE(status);
  for ( int i = 0; i < nkeys; i++ ) { 
    if ( vals[i] != 1+2 ) { go_BYE(-1); }
  }
  //------------------------------------------------
  t_stop = RDTSC();
  fprintf(stderr, "Passsed  %s %" PRIu64 "\n", __func__, (t_stop-t_start));
BYE:
  free_if_non_null(keys);
  free_if_non_null(vals);
  free_if_non_null(locs);
  free_if_non_null(hashes);
  free_if_non_null(is_founds);
  return status;
}
//----------------------------------------------------------
int
main(void)
{
  int status = 0;

  status = test_basic();       cBYE(status);
  status = test_add_a_lot();   cBYE(status);
  status = test_incr();        cBYE(status);
  status = test_getn_simple(); cBYE(status);
  status = test_setn(); cBYE(status);
  puts("ok");
BYE:
  return status;
}
