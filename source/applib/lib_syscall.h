/***
 * 
 * 系统调用实现相关头文件
 * 
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "lib_syscall_t.h"
#include "common/types.h"
#include "core/syscall.h"
#include "os_cfg.h"

/**
 * @brief 执行系统调用
 */
static inline 
int sys_call (syscall_args_t * args) {
	uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};
    __asm__ __volatile__(
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"
        :
        :[arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1),
         [arg0]"r"(args->arg0), [id]"r"(args->id), [a]"r"(addr)
    );
}


/**
 * @brief
 */
static inline
int msleep(int ms) {
    if(ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_sleep;
    args.arg0 = ms;

    return sys_call(&args);
}

#endif // LIB_SYSCALL_H