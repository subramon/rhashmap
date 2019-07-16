/*
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include "_q_rhashmap_I8_I8.h"
#include "_invariants_I8_I8.h"
#define VALTYPE  int64_t
#define KEYTYPE uint64_t
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
test_random(
    void
    )
{
  //C \section{Random Inserts and Deletes Test}
  //C Simplest test.
  int status = 0;
  int np = 0; // num_probes
  q_rhashmap_I8_I8_t *hmap = NULL;
  uint64_t t_stop, t_start = RDTSC();
  bool is_found;
  int niters = 64 * 1048576;
  KEYTYPE maxkey = UINT_MAX - 1;
  VALTYPE chk_old_val;
  VALTYPE new_val;
  uint32_t old_nitems;
  int num_choices = 9;

  srand48(t_start);
  srandom(RDTSC());
  //C \begin{itemize}
  //C \item Create a hashmap with default size 0. 
  hmap = q_rhashmap_create_I8_I8(0);
  status = invariants_I8_I8(hmap); cBYE(status);
  //C Should succeed.
  if ( hmap == NULL) { go_BYE(-1); }

  int num0 = 0, num1 = 0, num2 = 0, num3 = 0;
  //C \item Perform the following several times. Each time, pick one
  //C of the 4 options
  //C \begin{itemize}
  for( int iters = 0; iters < niters; iters++ ) { 
    KEYTYPE key = ( lrand48() % maxkey ) + 1;
    VALTYPE val = 123, old_val = 123;
    int choice = random() % num_choices; 
    unsigned int num_probes = 0;
    switch ( choice ) { 
      case 0 : 
      case 4 : 
        num0++;
        //C \item Case 0 
        //C \begin{itemize}
        //C \item Create a key that does not exist.
        for ( ; ; ) { 
          status = q_rhashmap_get_I8_I8(hmap, key, &val, &is_found); cBYE(status);
          if ( !is_found ) { break; }
          key++;
          num_probes++;
          if ( key > maxkey ) { key = 1; }
        }
        val = lrand48()  & 0x7FFFFFFF;
        old_nitems = hmap->nitems;
        //C \item Put this key in 
        status = q_rhashmap_put_I8_I8(hmap, key, ++val, Q_RHM_SET, &old_val, &np);
        cBYE(status);
        //C \item Verify that \verb+old_val = 0+
        if ( hmap->nitems != old_nitems + 1 ) { go_BYE(-1); }
        //C \item Verify that number of items has increased 
        //C \item Get this key and verify that it exists with proper value
        status = q_rhashmap_get_I8_I8(hmap, key, &new_val, &is_found); 
        cBYE(status);
        if ( val != new_val ) { go_BYE(-1); }
        if ( !is_found ) { go_BYE(-1); }
        //C \end{itemize}
        break;
      case 1 : 
      case 5 : 
      case 6 : 
        num1++;
        //C \item Case 1 
        //C \begin{itemize}
        //C \item Find a key that does exist.
        if ( hmap->nitems > 0 ) { 
          is_found = false;
          unsigned int pos = random() % hmap->size;
          for ( ; num_probes < hmap->size; num_probes++ ) { 
            if ( hmap->buckets[pos].key != 0 ) { 
              key = hmap->buckets[pos].key;
              chk_old_val = hmap->buckets[pos].val;
              is_found = true;
              break;
            }
            pos++;
            if ( pos == hmap->size ) { pos = 0; }
          }
          if ( !is_found ) { 
            printf("hello world\n");
            go_BYE(-1); 
          }
          status = q_rhashmap_get_I8_I8(hmap, key, &val, &is_found); cBYE(status);
          if ( !is_found ) { 
            printf("hello world\n");
            go_BYE(-1); 
          }
          val = lrand48()  & 0x7FFFFFFF;
          old_nitems = hmap->nitems;
          //C \item Put this key in 
          status = q_rhashmap_put_I8_I8(hmap, key, ++val, Q_RHM_SET, &old_val, &np);
          cBYE(status);
          if ( hmap->nitems != old_nitems ) { go_BYE(-1); }
          if ( old_val != chk_old_val ) { go_BYE(-1); }
          //C \item Verify that number of items has NOT increased 
          //C \item Get this key and verify that it exists with proper value
          status = q_rhashmap_get_I8_I8(hmap, key, &new_val, &is_found); 
          cBYE(status);
          if ( val != new_val ) { go_BYE(-1); }
          if ( !is_found ) { go_BYE(-1); }
        }
      //C \end{itemize}
       break;
      case 2 : 
      case 7 : 
      case 8 : 
        num2++;
        //C \item Case 2 
        //C \begin{itemize}
        //C \item Find a key that does not exist.
        for ( ; ; ) { 
          status = q_rhashmap_get_I8_I8(hmap, key, &val, &is_found); cBYE(status);
          if ( !is_found ) { break; }
          key = ( lrand48() % maxkey ) + 1;
        }
        old_nitems = hmap->nitems;
        //C \item Delete the key
        status = q_rhashmap_del_I8_I8(hmap, key, &val, &is_found); cBYE(status);
        //C \item Verify that number of items has NOT changed
        if ( old_nitems != hmap->nitems ) { go_BYE(-1); }
        //C \end{itemize}
        break;
      case 3 : 
        //C \item Case 3 
        num3++;
        //C \begin{itemize}
        //C \item Find a key that does exist.
        if ( hmap->nitems > 0 ) {
          is_found = false;
          unsigned int pos = random() % hmap->size;
          for ( ; num_probes < hmap->size; num_probes++ ) { 
            if ( hmap->buckets[pos].key != 0 ) { 
              key = hmap->buckets[pos].key;
              chk_old_val = hmap->buckets[pos].val;
              is_found = true;
              break;
            }
            pos++;
            if ( pos == hmap->size ) { pos = 0; }
          }
          if ( !is_found ) { 
            printf("hello world\n");
            go_BYE(-1); 
          }
          status = q_rhashmap_get_I8_I8(hmap, key, &val, &is_found); cBYE(status);
          if ( !is_found ) { 
            printf("hello world\n");
            go_BYE(-1); 
          }
          val = lrand48()  & 0x7FFFFFFF;
          old_nitems = hmap->nitems;
          //C \item Delete this key
          status = q_rhashmap_del_I8_I8(hmap, key, &val, &is_found); cBYE(status);
          //C \item Verify that number of items has decreased by 1
          if ( hmap->nitems != old_nitems -1  ) { go_BYE(-1); }
          //C \item Get this key and verify that it does not exist
          status = q_rhashmap_get_I8_I8(hmap, key, &new_val, &is_found); 
          cBYE(status);
          if ( is_found ) { go_BYE(-1); }
        }
      //C \end{itemize}
       break;
      default : 
       go_BYE(-1);
       break;
    } 
    //C \end{itemize}
  }
  status = invariants_I8_I8(hmap); cBYE(status);
  //C \item Destroy the hashmap
  q_rhashmap_destroy_I8_I8(hmap);
  t_stop = RDTSC();
  //C \end{itemize}
  fprintf(stdout, "Passsed  %s in cycles = %" PRIu64 "\n", __func__, (t_stop-t_start));
  fprintf(stdout, "0/1/2/3 = %d, %d, %d, %d \n", num0, num1, num2, num3);
BYE:
  return status;
}
int main(void)
{
  int status = 0;
  status = test_random();       cBYE(status);
BYE:
  return status;
}
