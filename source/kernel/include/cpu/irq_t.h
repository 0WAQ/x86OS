/**
 * 
 * 中断相关基本类型
 * 
 */
#ifndef IRQ_T_H
#define IRQ_T_H

#include "common/types.h"

typedef struct _exception_frame_t{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags;
}exception_frame_t;


#endif // IRQ_T_H