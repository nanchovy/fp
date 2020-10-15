#ifdef NVHTM
#error "NVHTM is defined!"
#endif
#include "tree.h"
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

#define ALLOCATING_BYTES 576

int main(int argc, char *argv[]) {
  struct rusage start, finish, result;
  struct timeval total;
  result.ru_utime.tv_sec = result.ru_utime.tv_sec = total.tv_sec = 0;
  result.ru_stime.tv_sec = result.ru_stime.tv_usec = total.tv_usec = 0;

  int loop_loop_times = 200000;
  int loop_times = 50;

  // allocatorとMemoryRootの初期化(一回目)
  initAllocator(NULL, "/mnt/nvmm/nagayasu/data", ALLOCATING_BYTES * (loop_times + 5) + 100, 1);
  root_allocate(sizeof(ppointer) + (64 - sizeof(ppointer) % 64), sizeof(PersistentLeafNode));

  for (int j = 1; j <= loop_loop_times; j++) {

    for (int i = 1; i <= loop_times; i++) {

      // pst_mem_allocate関数の実行にかかる時間の測定
      getrusage(RUSAGE_SELF, &start);
      void* new_pleaf_p1 = karmalloc(ALLOCATING_BYTES);
      // printfreelist();
      getrusage(RUSAGE_SELF, &finish);
      result.ru_utime.tv_sec += finish.ru_utime.tv_sec - start.ru_utime.tv_sec;
      result.ru_stime.tv_sec += finish.ru_stime.tv_sec - start.ru_stime.tv_sec;

      if (finish.ru_utime.tv_sec == start.ru_utime.tv_sec) {
        result.ru_utime.tv_usec += finish.ru_utime.tv_usec - start.ru_utime.tv_usec;
      } else {
        // printf("time!u: %ld, %ld\n", start.ru_stime.tv_usec, finish.ru_stime.tv_usec);
        result.ru_utime.tv_usec += (1000000 - start.ru_utime.tv_usec) + finish.ru_utime.tv_usec;
      }

      if (finish.ru_stime.tv_sec == start.ru_stime.tv_sec) {
        result.ru_stime.tv_usec += finish.ru_stime.tv_usec - start.ru_stime.tv_usec;
      } else {
        // printf("time!s: %ld, %ld\n", start.ru_stime.tv_usec, finish.ru_stime.tv_usec);
        result.ru_stime.tv_usec += (1000000 - start.ru_stime.tv_usec) + finish.ru_stime.tv_usec;
      }
    }

    destroyAllocator();
    resetFreeListHead();

  // allocatorとMemoryRootの初期化(二回目以降)
    initAllocator(NULL, "/mnt/nvmm/nagayasu/data", ALLOCATING_BYTES * (loop_times + 5) + 100, 1);
    root_allocate(sizeof(ppointer) + (64 - sizeof(ppointer) % 64), sizeof(PersistentLeafNode));
  }

  // usr time と sys timeの出力
  result.ru_utime.tv_sec += result.ru_utime.tv_usec / 1000000;
  result.ru_stime.tv_sec += result.ru_stime.tv_usec / 1000000;
  result.ru_utime.tv_usec %= 1000000;
  result.ru_stime.tv_usec %= 1000000;
  printf("%ld.%06ld, ", result.ru_utime.tv_sec, result.ru_utime.tv_usec);
  printf("%ld.%06ld, ", result.ru_stime.tv_sec, result.ru_stime.tv_usec);

  // 合計時間の出力
  total.tv_sec = result.ru_utime.tv_sec + result.ru_stime.tv_sec;
  total.tv_usec = result.ru_utime.tv_usec + result.ru_stime.tv_usec;
  total.tv_sec += total.tv_usec / 1000000;
  total.tv_usec %= 1000000;
  printf("%ld.%06ld\n", total.tv_sec, total.tv_usec);

  return 0;
}
