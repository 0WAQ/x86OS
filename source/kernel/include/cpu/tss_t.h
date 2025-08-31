#ifndef TSS_T_H
#define TSS_T_H

#include "common/types.h"

/**
 * @brief 任务状态段
 */
typedef struct _tss_t {
    u32_t pre_link;
    u32_t esp0, ss0, esp1, ss1, esp2, ss2;
    u32_t cr3;
    u32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u32_t es, cs, ss, ds, fs, gs;
    u32_t ldt;
    u32_t iomap;     // 控制IO访问相关信息
    // u32_t ssp;
} __attribute__((packed)) tss_t;

#endif // TSS_T_H