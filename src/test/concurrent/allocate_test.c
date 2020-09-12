#ifdef NVHTM
#error "NVHTM is defined!"
#endif
#include "tree.h"
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
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
  for (int i = 1; i <= 10; i++) {
    printfreelist();
    ppointer new_pleaf_p = pst_mem_allocate(16, 1);
    if (i % 3 == 0) {
      pst_mem_free(new_pleaf_p, 1, 1);
    }
  }

  showTree(bpt, 1);

  return 0;
}
