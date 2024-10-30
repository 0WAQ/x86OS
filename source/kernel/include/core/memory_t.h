/***
 * 
 * 内存管理相关的基本类型
 * 
 */
#ifndef MEMORY_T_H
#define MEMORY_T_H

#include "common/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"

#define MEM_EXT_START       (1024*1024)
#define MEM_EXT_END         (128 * 1024 * 1024 - 1) // 0x08000000, 目前qemu中设置的内存大小
#define MEM_PAGE_SIZE       (4096)
#define MEM_EBDA_START      (0x80000)
#define MEMORY_TASK_BASE    (0x80000000)    // 用户空间的起始虚拟地址, 以下是内核

/**
 * @brief 物理内存分配与释放的管理结构
 */
typedef struct _addr_alloc_t {

    bitmap_t bitmap;    // 辅助分配的位图

    uint32_t start;     // 内存块的起始地址
    uint32_t size;      // 内存块的大小
    uint32_t page_size; // 页大小, 页数 = size / page_size

    mutex_t mutex;      // 互斥锁    

}addr_alloc_t;

/**
 * @brief 内存映射结构
 * 表示一段虚拟地址与哪一段物理地址建立联系, 并设置权限
 */
typedef struct _memory_map_t{
    void* vstart;       // 线性地址的起始地址
    void* vend;         // 线性地址的结束地址
    void* pstart;       // 物理地址的起始地址
    uint32_t perm;
}memory_map_t;

#endif // MEMORY_T_H
