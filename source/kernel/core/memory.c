/***
 * 
 * 内存管理
 * 
 */
#include "core/memory.h"
#include "tools/log.h"
#include "tools/klib.h"

/**
 * @brief 初始化地址分配结构, 由调用者检查start和size的页边界
 * @param bits 位图的起始地址
 * @param start 内存起始地址
 * @param size 内存大小
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

/**
 * @brief 计算当前使用的内存空间总大小
 */
static uint32_t total_mem_size(boot_info_t* boot_info) {
    uint32_t tot = 0;
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        tot += boot_info->ram_region_cfg[i].size;
    }
    return tot;
}

// 物理内存分配器
static addr_alloc_t paddr_alloc;

void memroy_init(boot_info_t* boot_info) {
    log_print("...Memory Init...");
    show_mem_info(boot_info);

    // 计算1MB以上的空闲内存容量, 并向下对齐到页大小的整数倍
    uint32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START;
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE);
    log_print("Free Memory: start = 0x%x, size = 0x%x", MEM_EXT_START, mem_up1MB_free);

    // 链接脚本定义的变量, 其为内核结束后的第一个字节
    extern uint8_t* mem_free_start;
    uint8_t* mem_free = (uint8_t*)&mem_free_start;

    // 4GB总共需要 4*1024*1024*1024 / 4096 / 8 = 128KB大小的位图, 将其放在1MB字节以下
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    ASSERT(mem_free < (uint8_t*)MEM_EBDA_START);
}

void show_mem_info(boot_info_t* boot_info) {
    log_print("Memory Region:");
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        log_print("  [%d]: 0x%x - 0x%x", i, 
            boot_info->ram_region_cfg[i].start, 
            boot_info->ram_region_cfg[i].size);
    }
    log_print("");
}