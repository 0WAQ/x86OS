/**
 * 
 * 中断相关基本类型
 * 
 */
#ifndef IRQ_T_H
#define IRQ_T_H

#include "common/types.h"

#define IRQ0_DE     0

typedef struct _exception_frame_t{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t num, errno, eip, cs, eflags;
}exception_frame_t;


// 回调函数类型
typedef void (*irq_handler_t)();

#endif // IRQ_T_H