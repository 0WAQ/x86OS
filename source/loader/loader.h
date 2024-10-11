/**
 *
 * 二级加载部分，用于实现更为复杂的初始化、内核加载的工作。
 *
 */
#ifndef LOADER_H
#define LOADER_H

#include "common/types.h"
#include "common/boot_info.h"

/**
 * @brief 引导器入口函数
 */
void loader_entry();

/**
 * @brief 内存检测函数
 */
void detect_memory();

/**
 * @brief 显示字符串 
 */
void show_msg(const char* msg);

/**
 * @brief 内存检测信息结构
 */
typedef struct SMAP_entry {
    uint32_t base_low;      // 该块内存起始地址的低32位
    uint32_t base_high;     // 该块内存起始地址的高32位，未使用
    uint32_t length_low;    // 长度的低32位
    uint32_t length_high;   // 长度的高32位，未使用
    uint32_t type;          // 值为1表示可用RAM空间
    uint32_t ACPI;          // extended
}__attribute__((packed)) SMAP_entry_t;

#endif  // LOADER_H