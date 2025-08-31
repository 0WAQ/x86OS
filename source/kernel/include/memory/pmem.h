#ifndef PMEM_H
#define PMEM_H

#include "common/types.h"
#include "memory/memory_t.h"
#include "common/boot_info.h"


/**
 * @brief 初始化内存管理系统
 *  1. 初始化物理内存分配器: 将所有物理内存管理起来. 在1MB内存中分配物理位图
 *  2. 重新创建内核页表: 原loader中创建的页表已经不再适用
 */
void memroy_init(boot_info_t* boot_info);

u32_t pmem_alloc_page();
u32_t pmem_alloc_pages(u32_t nr);
void pmem_free_page(u32_t paddr);
void pmem_free_pages(u32_t pstart, u32_t nr);

/**
 * @brief 根据内存地址映射表, 建立内核页表
 */
static void create_kernel_table();

/**
 * @brief 初始化地址分配结构, 由调用者检查 start 和 size 的页边界
 */
static void paddr_alloc_init(paddr_alloc_t* alloc, u8_t* bits, u32_t start,
                            u32_t size, u32_t page_size);
static u32_t paddr_alloc_pages(paddr_alloc_t* alloc, u32_t nr);
static void paddr_free_pages(paddr_alloc_t* alloc, u32_t addr, u32_t nr);

static u32_t total_pmem_size(boot_info_t* boot_info);
static void show_pmem_info(boot_info_t* boot_info);

#endif // PMEM_H
