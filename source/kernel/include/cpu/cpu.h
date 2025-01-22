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
void set_segment_desc(int selector, uint32_t base, uint32_t limit, uint16_t attr);

/**
 * @brief 在IDT中设置中断门描述符
 */
void set_gate_desc(gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr);

/**
 * @brief 寻找空闲的表项
 */
int gdt_alloc_desc();

/**
 * @brief 释放表项
 */
void gdt_free_desc(int tss_sel);

/**
 * @brief
 */
void switch_to_tss(int tss_sel);

#endif // CPU_H