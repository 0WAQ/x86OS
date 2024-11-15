/***
 * 
 * 系统调用的外部相关头文件
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
    int ret;
    __asm__ __volatile__(
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"
        :"=a"(ret)  // 返回值通过eax寄存器传递给ret
        :[arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1),
         [arg0]"r"(args->arg0), [id]"r"(args->id), [a]"r"(addr)
    );
    return ret;
}


/**
 * @brief 让当前进程进入sleep
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

/**
 * @brief 获取当前进程的pid
 */
static inline
int getpid() {
    syscall_args_t args;
    args.id = SYS_getpid;
    return sys_call(&args);
}

/**
 * @brief 打印, 临时
 */
static inline
int print(const char* fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_print;
    args.arg0 = (uint32_t)fmt;
    args.arg1 = arg;
    return sys_call(&args);
}

/**
 * @brief 创建子进程
 */
static inline
int fork() {
    syscall_args_t args;
    args.id = SYS_fork;
    return sys_call(&args);
}

/**
 * @brief
 */
static inline
int execve(const char* path, char* const* argv, char* const* env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.arg0 = (int)path;
    args.arg1 = (int)argv;
    args.arg2 = (int)env;
    return sys_call(&args);
}

/**
 * @brief 让进程主动释放cpu
 */
static inline
int yield() {
    syscall_args_t args;
    args.id = SYS_yield;
    return sys_call(&args); 
}

#endif // LIB_SYSCALL_H