/***
 * 
 * 内存管理相关头文件
 * 
 */
#ifndef MEMORY_H
#define MEMORY_H

#include "memory_t.h"
#include "common/boot_info.h"

/**
 * @brief 初始化内存管理系统
 * 1. 初始化物理内存分配器: 将所有物理内存管理起来. 在1MB内存中分配物理位图
 * 2. 重新创建内核页表: 原loader中创建的页表已经不再适用
 */
void memroy_init(boot_info_t* boot_info);

/**
 * @brief 打印内存空间
 */
void show_mem_info(boot_info_t* boot_info);

#endif // MEMORY_H
