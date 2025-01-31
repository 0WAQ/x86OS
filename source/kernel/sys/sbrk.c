#include "sys/syscall.h"

char* sys_sbrk(int incr) {
    
    task_t* task = get_curr_task();
    char* pre_head_end = (char*)task->heap_end;
    int pre_incr = incr;

    // TODO: 暂不处理incr小于0的情况
    ASSERT(incr >= 0);

    // 从原堆的结束地址开始分配
    u32_t alloc_start = task->heap_end;  // 从该地址处开始分配
    u32_t alloc_end = alloc_start + incr;

    if(incr == 0) {
        goto sys_sbrk_normal;
    }

    u32_t start_offset = alloc_start % MEM_PAGE_SIZE;  // alloc_start在一页中的偏移量
    
    // 若偏移量为0, 则要么heap_end为0, 要么为页的倍数, 相当于没有空闲的页了, 则直接去按页分配
    if(start_offset != 0) {
        // 若加上incr不超过一页, 则就在本页面内分配
        if(start_offset + incr <= MEM_PAGE_SIZE) {
            goto sys_sbrk_normal;
        }
        else {  // 若超过一页, 先将本页面剩余的空间分配出去, 在去下面按页分配
            u32_t curr_size = MEM_PAGE_SIZE - start_offset;
            alloc_start += curr_size;
            incr -= curr_size;
        }
    }

    // 按页分配
    if(incr != 0) {
        u32_t curr_size = alloc_end - alloc_start;
        int ret = memory_alloc_page_for(alloc_start, curr_size, PTE_P | PTE_U | PTE_W);
        if(ret < 0) {
            log_print("sbrk: alloc mem failed.");
            goto sys_sbrk_failed;
        }
    }

sys_sbrk_normal:
    log_print("sbrk(%d), end=0x%x", pre_incr, alloc_end);
    task->heap_end = alloc_end;
    return (char*)pre_head_end;

sys_sbrk_failed:
    return (char*)-1;
}