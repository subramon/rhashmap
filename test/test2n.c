/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

// gcc -g  $QC_FLAGS test2n.c ../lua/libtest2.so -I../xgen_inc/ -I../inc/
// TODO: Write tests on cnt 
#include "_hmap_chk.h"
#include "_hmap_chk_no_holes.h"
#include "_hmap_create.h"
#include "_hmap_del.h"
#include "_hmap_destroy.h"
#include "_hmap_mk_hsh.h"
#include "_hmap_mk_loc.h"
#include "_hmap_mk_tid.h"
#include "_hmap_putn.h"
#include "_hmap_get.h"
#include "_hmap_getn.h"

static int
chk_parallelization(
    uint8_t *tids, 
    int N,
    int nT
    )
{
  int status = 0;
  int *work_per_thread = NULL;
  work_per_thread = malloc(nT * sizeof(int));
  for ( int i = 0; i < nT; i++ ) { work_per_thread[i] = 0; }
  for ( int i = 0; i < N; i++ ) { 
    int tid = tids[i];
    if ( ( tid < 0 ) || ( tid >= nT ) ) { go_BYE(-1); }
    work_per_thread[tid]++;
  } for ( int i = 0; i < nT; i++ ) { 
    fprintf(stderr, "tid %d: work %d \n", i,  work_per_thread[i]);
  }
  free_if_non_null(work_per_thread);
BYE:
  return status;
}
static int
test_basic(
    void
    )
{
  int status = 0;
  hmap_t *ptr_hmap = NULL;
  val_t *vals = NULL;
  keytype  *keys = NULL;
  uint32_t *hshs = NULL;
  uint32_t *locs = NULL;
  uint8_t  *tids = NULL;
  uint8_t  *fnds = NULL;
  uint32_t N = 100000;
  uint64_t num_probes;// for diagnostics
  uint32_t num_new; // for diagnostics
  int nT = omp_get_max_threads();

  vals = malloc(N * sizeof(val_t));
  keys = malloc(N * sizeof(keytype));
  hshs = malloc(N * sizeof(uint32_t));
  locs = malloc(N * sizeof(uint32_t));
  tids = malloc(N * sizeof(uint8_t));
  fnds = malloc(N * sizeof(uint8_t));

  ptr_hmap = hmap_create(0);
  status = hmap_chk(ptr_hmap); cBYE(status);
  if ( ptr_hmap == NULL) { go_BYE(-1); }
  fprintf(stderr, "Put same key with values = 1, 2, ... N\n");
  for ( uint32_t i = 0; i < N; i++ ) { keys[i] = 1; }
  uint64_t sum = 0;
  for ( uint32_t i = 0; i < N; i++ ) { sum += i+1; vals[i].val_1 = i+1; }
  status = hmap_mk_hsh(keys, N, ptr_hmap->hashkey, hshs);
  status = hmap_mk_loc(hshs, N, ptr_hmap->size, locs);
  status = hmap_mk_tid(hshs, N, nT, tids);
  chk_parallelization(tids, N, nT); 

  status = hmap_putn(ptr_hmap, keys, hshs, locs, tids, nT,
       vals, N, fnds, &num_new, &num_probes);
  cBYE(status);
  status = hmap_chk(ptr_hmap); cBYE(status);
  if ( ptr_hmap->nitems != 1 ) { go_BYE(-1); }
  if ( num_new != N ) { go_BYE(-1); }
  val_t altval;
  cnttype cnt;
  bool is_found;
  status = hmap_get(ptr_hmap, keys[0], &altval, &cnt, &is_found, &num_probes);
  cBYE(status);
  if ( altval.val_1 != sum ) { go_BYE(-1); }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
  fprintf(stderr, "Put key = N+1, N+2 with values 1 \n");
  for ( uint32_t i = 0; i < N; i++ ) { keys[i] = N+i; }
  for ( uint32_t i = 0; i < N; i++ ) { vals[i].val_1 = 1; }
  status = hmap_mk_hsh(keys, N, ptr_hmap->hashkey, hshs);
  status = hmap_mk_loc(hshs, N, ptr_hmap->size, locs);
  status = hmap_mk_tid(hshs, N, nT, tids);
  chk_parallelization(tids, N, nT); 
  status = hmap_putn(ptr_hmap, keys, hshs, locs, tids, nT,
       vals, N, fnds, &num_new, &num_probes);
  cBYE(status);
  if ( ptr_hmap->nitems != N+1 ) { go_BYE(-1); }
  if ( num_new != N ) { go_BYE(-1); }
  for ( uint32_t i = 0; i < N; i++ ) { 
    status = hmap_get(ptr_hmap, keys[i], &altval, &cnt, &is_found, &num_probes);
   if ( altval.val_1 != 1 ) { go_BYE(-1); }
  }
  cBYE(status);
  status = hmap_mk_hsh(keys, N, ptr_hmap->hashkey, hshs);
  status = hmap_mk_loc(hshs, N, ptr_hmap->size, locs);
  status = hmap_mk_tid(hshs, N, nT, tids);
  status = hmap_getn (ptr_hmap, nT, keys, locs, vals, N, fnds);
  cBYE(status);
  for ( uint32_t i = 0; i < N; i++ ) { 
    if ( vals[i].val_1 != 1 ) { go_BYE(-1); }
    if ( fnds[i] == false ) { go_BYE(-1); }
  }
  status = hmap_chk_no_holes(ptr_hmap);  cBYE(status);
    

BYE:
  hmap_destroy(ptr_hmap); 
  int bak_status = hmap_chk(ptr_hmap); 
  if ( bak_status < 0 ) { WHEREAMI; }
  free_if_non_null(keys);
  free_if_non_null(hshs);
  free_if_non_null(locs);
  free_if_non_null(tids);
  free_if_non_null(fnds);
  return status;
}

int main(void)
{
  int status = 0;
  status = test_basic(); cBYE(status);
  printf("Success\n");
BYE:
  return status;
}
