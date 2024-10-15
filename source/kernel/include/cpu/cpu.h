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
 * @brief 在GDT中设置段描述符
 */
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

/**
 * @brief 在IDT中设置中断门描述符
 */
void gate_desc_set(gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr);

#endif // CPU_H