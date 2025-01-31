/***
 * 
 * 内存管理相关头文件
 * 
 */
#ifndef MEMORY_H
#define MEMORY_H

#include "memory_t.h"
#include "cpu/mmu.h"
#include "common/boot_info.h"

/**
 * @brief 初始化内存管理系统
 * 1. 初始化物理内存分配器: 将所有物理内存管理起来. 在1MB内存中分配物理位图
 * 2. 重新创建内核页表: 原loader中创建的页表已经不再适用
 */
void memroy_init(boot_info_t* boot_info);

/**
 * @brief 根据内存映射表, 构造内核页表
 */
static void create_kernel_table();

/**
 * @brief 只处理一个地址映射关系
 */
static int memory_create_map(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t page_nr, u32_t perm);

/**
 * @brief 创建用户虚拟内存空间
 */
u32_t memory_create_uvm();

/**
 * @brief 为vaddr分配页
 */
int memory_alloc_page_for(u32_t vaddr, u32_t size, u32_t perm);

/**
 * @brief 子函数, 指定在一个页目录表中取分配
 */
int _memory_alloc_page_for(u32_t page_dir, u32_t vaddr, u32_t size, u32_t perm);

/**
 * @brief 分配一页内存
 */
u32_t memory_alloc_page();

/**
 * @brief 释放一页内存
 */
void memory_free_page(u32_t addr);

/**
 * @brief 打印内存空间
 */
static void show_mem_info(boot_info_t* boot_info);


/**
 * @brief 初始化地址分配结构, 由调用者检查start和size的页边界
 */
static void addr_alloc_init(addr_alloc_t* alloc, u8_t* bits, 
            u32_t start, u32_t size, u32_t page_size)
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
static u32_t addr_alloc_page(addr_alloc_t* alloc, int page_count) {
    u32_t addr = 0;

    mutex_lock(&alloc->mutex);

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
static void addr_free_page(addr_alloc_t* alloc, u32_t addr, int page_count) {
    mutex_lock(&alloc->mutex);

    u32_t page_index = (addr - alloc->start) / alloc->page_size;
    // 将这段内存页设置为0
    bitmap_set_bit(&alloc->bitmap, page_index, page_count, 0);

    mutex_unlock(&alloc->mutex);
}

/**
 * @brief 计算当前使用的内存空间总大小
 */
static u32_t total_mem_size(boot_info_t* boot_info) {
    u32_t tot = 0;
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        tot += boot_info->ram_region_cfg[i].size;
    }
    return tot;
}

/**
 * @brief 返回当前使用的页目录表
 */
static pde_t* curr_page_dir() {
    return (pde_t*)(get_curr_task()->tss.cr3);
}

/**
 * @brief 为子进程拷贝父进程的代码和数据
 */
u32_t memory_copy_uvm(u32_t page_dir, u32_t to_page_dir);

/**
 * @brief 销毁子进程创建过的代码和数据
 */
void memory_destory_uvm(u32_t page_dir);

/**
 * @brief 将vaddr通过page_dir转换成物理地址
 */
u32_t memory_vaddr_to_paddr(u32_t page_dir, u32_t vaddr);

/**
 * @brief 在不同页表间拷贝数据
 */
int memory_copy_uvm_data(u32_t to, u32_t page_dir, u32_t from, u32_t size);

#endif // MEMORY_H
