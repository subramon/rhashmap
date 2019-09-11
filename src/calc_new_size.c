//START_INCLUDES
#include "hmap_common.h"
//STOP_INCLUDES
#include "_calc_new_size.h"
/* Checks whether resize is needed. If so, calculates newsize */
/* Resize needed when occupancy is too high or too low */
//START_FUNC_DECL
int
calc_new_size(
    uint32_t nitems, 
    uint32_t minsize, 
    uint32_t size, 
    bool decreasing, 
    /* true =>  just added an element and are concerned about sparsity
     * false=> just added an element and are concerned about denseness
    */
    uint32_t *ptr_newsize,
    bool *ptr_resize
    )
//STOP_FUNC_DECL
{
  int status = 0;
  *ptr_resize = false;
  uint64_t newsize = 0;
  uint64_t threshold;
  if ( decreasing ) { 
    /*
     * If the load factor is less than threshold, then shrink by
     * halving the size, but not more than the minimum size.
     */
    threshold = (uint64_t)(LOW_WATER_MARK * size);
    if ( ( nitems > minsize ) && ( nitems < threshold ) ) {
      *ptr_resize = true;
      newsize = MAX(size >> 1, minsize);
    }
  }
  else {
    // If the load factor is more than the threshold, then resize.
    threshold = (uint64_t)(HIGH_WATER_MARK * (double)size);
    // TODO P4 Clean up the following code 
    if ( nitems > threshold ) { 
      *ptr_resize = true;
      for ( ; nitems > threshold; ) { 
        /*
         * Grow the hash table by doubling its size, but with
         * a limit of MAX_GROWTH_STEP.
         */
        uint64_t grow_limit = size + MAX_GROWTH_STEP;
        newsize = MIN(size << 1, grow_limit);
        threshold = (uint64_t)(0.85 * (double)newsize);
      }
    }
  }
  if ( newsize > UINT_MAX ) { go_BYE(-1); }
  *ptr_newsize = newsize;
BYE:
  return status;
}
