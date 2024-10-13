/**
 * 
 * 与x86的体系结构相关的接口
 * 
 */
#ifndef CPU_H
#define CPU_H

#include "cpu_t.h"

/**
 * @brief 初始化cpu
 */
void cpu_init();

/**
 * @brief 初始化gdt
 */
void gdt_init();

/**
 * @brief 设置段描述符
 */
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);


#endif // CPU_H