/**
 *
 * 二级加载器16位和32位的通用部分，用于实现更为复杂的初始化、内核加载的工作。
 *
 */
#ifndef LOADER_H
#define LOADER_H

#include "common/types.h"
#include "common/boot_info.h"
#include "common/cpu_instr.h"

// 全局的启动信息参数，来自loader_16.c
extern boot_info_t boot_info;

/**
 * @brief 内存检测信息结构
 */
typedef struct SMAP_entry {
    u32_t base_low;      // 该块内存起始地址的低32位
    u32_t base_high;     // 该块内存起始地址的高32位，未使用
    u32_t length_low;    // 长度的低32位
    u32_t length_high;   // 长度的高32位，未使用
    u32_t type;          // 值为1表示可用RAM空间
    u32_t ACPI;          // extended
}__attribute__((packed)) SMAP_entry_t;


#define CR4_PSE         (1 << 4)
#define CR0_PG          (1 << 31)
#define PDE_P           (1 << 0)
#define PDE_W           (1 << 1)
#define PDE_PS          (1 << 7)

#endif  // LOADER_H