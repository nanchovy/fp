#ifndef CONCURRENT
// #  error "CONCURRENT is not defined!"
#endif
#include "allocator.h"

PAddr PADDR_NULL = { 0, 0 };

#ifdef ALLOCTIME
struct timespec myalloc_start_time = {0, 0};
struct timespec myalloc_finish_time = {0, 0};
long long allocate_time = 0;
long long persist_time = 0;
#endif

#define DEFAULT_NODE_NUM 0 // デフォルトでフリーリストに入っている数
#define NB_NODE_AT_ONCE 1

typedef struct FreeNode {
    struct FreeNode *next;
    void *node;
} FreeNode;

typedef struct MemoryRoot {
    unsigned char global_lock;
    void *global_free_area_head;
    // size_t remaining_amount;
    // FreeNode *global_free_list_head; // for after recovery
    // unsigned char **list_lock;
    // FreeNode ***local_free_list_head_ary; // [i][j] -> スレッドiの持つスレッドj用のフリーリストへのポインタ
    // FreeNode ***local_free_list_tail_ary; // [i][j] -> スレッドiの持つスレッドj用のフリーリスト末尾へのポインタ
} MemoryRoot;




/*
 * 構造：
 * | head_ppointer | root_ppointer | node[0] | ...
 * 新規作成の場合head_ppointer = {0, 0}
 * fidは1から．現在のところ複数ファイルには対応していない．
 * また最初に確保した以上のメモリを確保することもない．
 * nスレッドごとにn * nのフリーリストを持ち，ノードのスレッドidと同じリストに返す
 */

void *_pmem_mmap_head = NULL;
void *_pmem_user_head = NULL;
size_t _pmem_mmap_size = 0;
size_t _pmem_user_size = 0;
int _number_of_thread = -1;
MemoryRoot *_pmem_memory_root = NULL;
size_t _tree_node_size = 0;
unsigned char _initialized_by_others = 0;

// for karmalloc
typedef double ALIGN;

union Pheader {
    struct
    {
        union Pheader *ptr; // pointer of next block
        unsigned size;      // size program can use
    } s;
    ALIGN x;
};

typedef union Pheader PMemHeader;

PMemHeader base;
PMemHeader *allocp;
#define NALLOC 128

void initMemoryRoot(MemoryRoot *mr, unsigned char thread_num, void *head, size_t pmem_size, size_t node_size, FreeNode *global_list_head) {
    mr->global_lock = 0;
    mr->global_free_area_head = head;
}
void destroyMemoryRoot(MemoryRoot *mr) {
    // TODO: need to traverse free list
    return 0;
}

int isSamePAddr(PAddr a, PAddr b) {
    return (a.fid == b.fid) && (a.offset == b.offset);
}

ppointer getPersistentAddr(void *addr) {
    ppointer paddr;
    if (addr == NULL) {
        return PADDR_NULL;
    } else {
        paddr.fid = 1;  // for single thread
        paddr.offset = addr - _pmem_mmap_head;
        return paddr;
    }
}

void *getTransientAddr(ppointer paddr) {
    if (isSamePAddr(paddr, PADDR_NULL)) {
        return NULL;
    } else {
        return _pmem_mmap_head + paddr.offset;
    }
}

int initAllocator(void *existing_p, const char *path, size_t pmem_size, unsigned char thread_num) {
    _number_of_thread = thread_num;

    if (existing_p != NULL) {
        _initialized_by_others = 1;
        _pmem_mmap_head = existing_p;
        _pmem_mmap_size = pmem_size;
        _pmem_user_head = _pmem_mmap_head + sizeof(AllocatorHeader);
        _pmem_user_size = pmem_size - sizeof(AllocatorHeader);
        memset(_pmem_mmap_head, 0, _pmem_mmap_size);
        *(PAddr *)_pmem_mmap_head = PADDR_NULL;
        return 0;
    }

    struct stat fi;
    int err;
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        if (errno == ENOENT) {
            AllocatorHeader new_header;
            new_header.node_head = PADDR_NULL;
            fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            int written_size = write(fd, &new_header, sizeof(AllocatorHeader));
            if (written_size < sizeof(ppointer)) {
                perror("write");
                return -1;
            }
            lseek(fd, 0, SEEK_SET);
        }
        if (fd == -1) {
            perror("open");
            return -1;
        }
    }

    // extend file
    if (fstat(fd, &fi) == -1) {
        perror("fstat");
        return -1;
    }
    if (fi.st_size < pmem_size) {
        err = ftruncate(fd, pmem_size);
        if (err != 0) {
            perror("ftruncate");
            return -1;
        }
    }

    _pmem_mmap_head = mmap(NULL, pmem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (_pmem_mmap_head == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    _pmem_mmap_size = pmem_size;
    _pmem_user_head = _pmem_mmap_head + sizeof(AllocatorHeader);
    _pmem_user_size = pmem_size - sizeof(AllocatorHeader);
    memset(_pmem_mmap_head, 0, _pmem_mmap_size);


    // karmalloc
    *(PMemHeader *)_pmem_user_head = base;
    (PMemHeader *)_pmem_mmap_head->s.ptr = &base;
    (PMemHeader *)_pmem_mmap_head->s.size = _pmem_user_size - sizeof(PMemHeader);
    // base.s.ptr = &base;
    // base.s.size = _pmem_user_size - sizeof(PMemHeader);

    // fd can be closed after mmap
    err = close(fd);
    if (err == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

ppointer getHeadPPointer() {
    if (_pmem_user_head == NULL) {
        return PADDR_NULL;
    }

    ppointer *head_pointer = (ppointer *)_pmem_user_head;
    return *head_pointer;
}

ppointer recoverAllocator(ppointer (*getNext)(ppointer)) {
    // TODO: implement recovery
    // find free node and make global free list
    return getHeadPPointer();
};

ppointer *root_allocate(size_t size, size_t node_size) {
#ifdef ALLOCTIME
    clock_gettime(CLOCK_MONOTONIC, &myalloc_start_time);
#endif
    _pmem_memory_root = (MemoryRoot *)vol_mem_allocate(sizeof(MemoryRoot));
    initMemoryRoot(_pmem_memory_root, _number_of_thread, _pmem_user_head + size, _pmem_user_size - size, node_size, NULL);
#ifdef ALLOCTIME
    clock_gettime(CLOCK_MONOTONIC, &myalloc_finish_time);
    myalloc_allocate_time += (myalloc_finish_time.tv_sec - myalloc_start_time.tv_sec) * 1000000000L + (myalloc_finish_time.tv_nsec - myalloc_start_time.tv_nsec);
#endif
    _tree_node_size = node_size;
    ppointer *root_p = (ppointer *)_pmem_user_head;
    if (isSamePAddr(PADDR_NULL, ((AllocatorHeader*)_pmem_mmap_head)->node_head)) {
        ((AllocatorHeader *)_pmem_mmap_head)->node_head = getPersistentAddr(_pmem_user_head);
    }
    return root_p;
}

void root_free(ppointer *root) {
}

ppointer pst_mem_allocate(size_t size, unsigned char tid) {
    void *new_node;

    new_node = karmalloc(size);
    return getPersistentAddr(new_node);
}

void *karmalloc(size_t nbytes) {
    PMemHeader *p, *q;
    unsigned nunits;

    nunits = (nbytes + sizeof(PMemHeader) - 1) / sizeof(PMemHeader) + 1; // number of block this function looking for
    if ((q = allocp) == NULL) {
        // initialization
        base.s.ptr = allocp = q = &base;
        base.s.size = 0;
    }
    for (p = q->s.ptr;; q = p, p->s.ptr) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits) {
                // exactly
                q->s.ptr = p->s.ptr;
            } else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            allocp = q;
            return ((char *)(p + 1)); // return only data part (without header)
        }
        if (p == allocp && (p = morecore(nunits)) == NULL) {
            // when p returns to start block (in case there is not block which has enough memory)
            // TODO: implement morecore
            return (NULL);
        }
    }
}



void pst_mem_free(ppointer node, unsigned char node_tid, unsigned char tid) {
    node_tid--;
    tid--;
    FreeNode *new_free = (FreeNode *)vol_mem_allocate(sizeof(FreeNode));
    new_free->node = getTransientAddr(node);
    while (!__sync_bool_compare_and_swap(&_pmem_memory_root->list_lock[tid][node_tid], 0, 1))
	    _mm_pause();
    addToList(new_free, &_pmem_memory_root->local_free_list_head_ary[tid][node_tid], &_pmem_memory_root->local_free_list_tail_ary[tid][node_tid]);
    _pmem_memory_root->list_lock[tid][node_tid] = 0;
}

void *vol_mem_allocate(size_t size) {
    return malloc(size);
}

void vol_mem_free(void *p) {
    free(p);
}

int destroyAllocator() {
    destroyMemoryRoot(_pmem_memory_root);
    vol_mem_free(_pmem_memory_root);

#ifdef ALLOCTIME
    printf("alloctime:%lld.%09lld\n", myalloc_allocate_time/1000000000L, myalloc_allocate_time%1000000000L);
    printf("persisttime:%lld.%09lld\n", myalloc_persist_time/1000000000L, myalloc_persist_time%1000000000L);
#endif

    if (!_initialized_by_others) {
        int err = munmap(_pmem_mmap_head, _pmem_mmap_size);
        if (err == -1) {
            perror("munmap");
            return -1;
        }
    }
    return 0;
}
