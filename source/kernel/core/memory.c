/***
 * 
 * 内存管理
 * 
 */
#include "core/memory.h"
#include "tools/log.h"

/**
 * @brief 初始化地址分配结构, 由调用者检查start和size的页边界
 */
static void addr_alloc_init(addr_alloc_t* alloc, uint8_t* bits, 
            uint32_t start, uint32_t size, uint32_t page_size)
{
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, size / page_size, 0);
}

/**
 * @brief 分配多页内存
 */
static uint32_t addr_alloc_page(addr_alloc_t* alloc, int page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t addr = 0;

    // 分配连续的多个空闲页
    int page_index = bitmap_alloc_nbits(&alloc->bitmap, page_count, 0);
    if(page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size;
    }
    mutex_unlock(&alloc->mutex);

    return addr;
}

/**
 * @brief 释放多页内存
 */
static void addr_free_page(addr_alloc_t* alloc, uint32_t addr, int page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t page_index = (addr - alloc->start) / alloc->page_size;
    // 将这段内存页设置为0
    bitmap_set_bit(&alloc->bitmap, page_index, page_count, 0);

    mutex_unlock(&alloc->mutex);
}

void memroy_init(boot_info_t* boot_info) {
    addr_alloc_t addr_alloc;
    uint8_t bits[8];

    // 初始化地址分配结构, 该结构表示从地址0x1000开始的连续64个大小为4096的页
    addr_alloc_init(&addr_alloc, bits, 0x1000, 64*4096, 4096);

    // 在addr_alloc表示的地址空间中分配页
    for(int i = 0; i < 32; i++) {
        // 每次分配2个
        uint32_t addr = addr_alloc_page(&addr_alloc, 2);
        log_print("alloc addr: 0x%x", addr);
    }

    uint32_t addr = 0x1000;
    for(int i = 0; i < 32; i++) {
        addr_free_page(&addr_alloc, addr, 2);
        addr += 4096 * 2;
    }
}