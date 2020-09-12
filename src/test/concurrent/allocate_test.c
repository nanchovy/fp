#ifdef NVHTM
#error "NVHTM is defined!"
#endif
#include "tree.h"
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

int main(int argc, char *argv[])
{
  struct rusage start, finish;
  BPTree *bpt;
  KeyValuePair kv;
  int loop_times = 400;
  int max_val = 1000;
  if (argc > 1) {
    loop_times = atoi(argv[1]);
    if (loop_times <= 0) {
      printf("invalid argument\n");
      return 1;
    }
    max_val = atoi(argv[2]);
    if (max_val <= 0) {
      printf("invalid argument\n");
      return 1;
    }
  } else {
       printf("default: loop_times = 40, max_val = 1000\n");
  }
  initAllocator(NULL, "/mnt/nvmm/nagayasu/data", sizeof(PersistentLeafNode) * (loop_times * 2 / (MAX_PAIR / 2) + 1 + sizeof(AllocatorHeader)), 1);
  bpt = newBPTree();
  kv.key = 1;
  kv.value = 1;

  getrusage(RUSAGE_SELF, &start);
  for (int i = 1; i <= 10; i++) {
    ppointer new_pleaf_p1 = pst_mem_allocate(16, 1);
    ppointer new_pleaf_p2 = pst_mem_allocate(32, 1);
    ppointer new_pleaf_p3 = pst_mem_allocate(64, 1);
    ppointer new_pleaf_p4 = pst_mem_allocate(70, 1);
    pst_mem_free(new_pleaf_p3, 1, 1);
    printfreelist();
  }
  getrusage(RUSAGE_SELF, &finish);
  showTree(bpt, 1);

  printf("usr time: %ld.%d\n", finish.ru_utime.tv_sec - start.ru_utime.tv_sec, finish.ru_utime.tv_usec - start.ru_utime.tv_usec);
  printf("sys time: %ld.%d\n", finish.ru_stime.tv_sec - start.ru_stime.tv_sec, finish.ru_stime.tv_usec - start.ru_stime.tv_usec);

  return 0;
}
