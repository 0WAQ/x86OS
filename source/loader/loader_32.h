/**
 * 
 * 32位loader的头文件
 * 
 */
#ifndef LOADER_32_H
#define LOADER_32_H

#include "common/elf.h"

/**
 * @brief 二级引导器32位入口
 */
void loader_kernel();


/**
 * @brief 读磁盘，从磁盘上加载内核
 */
void read_disk(uint32_t sector, uint32_t sector_cnt, uint8_t* buffer);


/**
 * @brief 从指定内存中读取 elf 文件
 */
uint32_t reload_elf_file(uint8_t* file_buffer);


/**
 * @brief 死机
 */
void die(int err_code);

#endif // LOADER_32_H