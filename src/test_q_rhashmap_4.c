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
//----------------------------------------------------------
static int
test_multi_set(
    void
    )
{
  int status = 0;
  //C \section{Test multi-set}
  //C \label{multi_set}
  int np; // number of probes
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
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval, &np);
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
    status = q_rhashmap_put(hmap, keys[i], vals[i], Q_RHM_SET, &oldval, &np);
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
  fprintf(stdout, "Passsed  %s in cycles  =%" PRIu64 "\n", __func__, (t_stop-t_start));
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
  status = test_multi_set(); cBYE(status);
BYE:
  return status;
}
