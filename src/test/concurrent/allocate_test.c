#ifdef NVHTM
#error "NVHTM is defined!"
#endif
#include "tree.h"
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

#define ALLOCATING_BYTES 320

int main(int argc, char *argv[])
{
  struct rusage start, finish, result;
  result.ru_utime.tv_sec = result.ru_utime.tv_sec = 0;
  result.ru_stime.tv_usec = result.ru_stime.tv_usec = 0;

  BPTree *bpt;
  KeyValuePair kv;
  
  int loop_times = 400;
  int max_val = 1000;

  initAllocator(NULL, "/mnt/nvmm/nagayasu/data", 320 * loop_times + 100, 1);
  bpt = newBPTree();
  kv.key = 1;
  kv.value = 1;

  
  for (int i = 1; i <= loop_times; i++) {
    getrusage(RUSAGE_SELF, &start);
    ppointer new_pleaf_p1 = pst_mem_allocate(ALLOCATING_BYTES, 1);
    getrusage(RUSAGE_SELF, &finish);
    result.ru_utime.tv_sec += finish.ru_utime.tv_sec - start.ru_utime.tv_sec;
    result.ru_utime.tv_usec += finish.ru_utime.tv_usec - start.ru_utime.tv_usec;
    result.ru_stime.tv_sec += finish.ru_stime.tv_sec - start.ru_stime.tv_sec;
    result.ru_stime.tv_usec += finish.ru_stime.tv_usec - start.ru_stime.tv_usec;
  }

//   showTree(bpt, 1);

  destroyBPTree(bpt, 1);
  destroyAllocator();


  printf("usr time: %ld.%06d\n", finish.ru_utime.tv_sec - start.ru_utime.tv_sec, finish.ru_utime.tv_usec - start.ru_utime.tv_usec);
  printf("sys time: %ld.%06d\n", finish.ru_stime.tv_sec - start.ru_stime.tv_sec, finish.ru_stime.tv_usec - start.ru_stime.tv_usec);

  return 0;
}
