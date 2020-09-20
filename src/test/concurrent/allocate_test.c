#ifdef NVHTM
#  error "NVHTM is defined!"
#endif
#include "tree.h"
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

int main(int argc, char *argv[]) {
    BPTree *bpt;
    KeyValuePair kv;
    int loop_loop_times = 10000;
    int loop_times = 320;
    int max_val = 1000;
    struct rusage start, finish;
    struct timeval user_time, sys_time;
    user_time.tv_sec = 0;
    user_time.tv_usec = 0;
    sys_time.tv_sec = 0;
    sys_time.tv_usec = 0;

    for (int j = 1; j <= loop_times; j++) {
        initAllocator(NULL, "/mnt/nvmm/nagayasu/data", sizeof(PersistentLeafNode) * (loop_times * 2 / (MAX_PAIR / 2) + 1 + sizeof(AllocatorHeader)), 1);
        bpt = newBPTree();
        kv.key = 1;
        kv.value = 1;
        for (int i = 1; i <= loop_times; i++) {
            kv.key = i;
            printf("insert %ld\n", kv.key);
            if (insert(bpt, kv, 1)) {
                printf("success\n");
            } else {
                printf("failure\n");
            }
        }
        showTree(bpt, 1);
        destroyBPTree(bpt, 1);
        destroyAllocator();
    }
    printf("評価結果\n");
    printf("%ld, %ld, %ld, %ld\n", user_time.tv_sec, user_time.tv_usec, sys_time.tv_sec, sys_time.tv_usec);
    return 0;
}
