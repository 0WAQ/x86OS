/**
 * 
 * 系统调用基本类型
 * 
 */
#ifndef SYSCALL_T_H
#define SYSCALL_T_H

#include "common/types.h"

#define SYSCALL_PARAM_COUNT     5

// 系统调用号
#define SYS_sleep       0
#define SYS_getpid      1
#define SYS_fork        2
#define SYS_print       100


typedef int(*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t);

/**
 * @brief 执行系统调用时的栈帧
 */
typedef struct _syscall_frame_t {
    uint32_t eflags;
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, dummy_esp, ebx, edx, ecx, eax;

    // 以下是在切换栈时cpu压入的参数
    uint32_t eip, cs;
    uint32_t id, arg0, arg1, arg2, arg3;
    uint32_t esp, ss;
}syscall_frame_t;

#endif // SYSCALL_T_H