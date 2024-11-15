/***
 * 
 * 系统调用的外部相关头文件
 * 
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "lib_syscall_t.h"

/**
 * @brief 系统调用的统一入口
 */
static inline
int sys_call(syscall_args_t * args);

/**
 * @brief 让当前进程进入sleep
 */
int msleep(int ms);

/**
 * @brief 获取当前进程的pid
 */
int getpid();

/**
 * @brief 打印, 临时 TODO:
 */
int print(const char* fmt, int arg);

/**
 * @brief 创建子进程
 */
int fork();

/**
 * @brief 从文件中执行新进程
 */
int execve(const char* path, char* const* argv, char* const* env);

/**
 * @brief 让进程主动释放cpu
 */
int yield();

/**
 * @brief 打开文件
 */
int open(const char* filename, int flags, ...);

/**
 * @brief 读文件
 */
int read(int fd, char* buf, int len);

/**
 * @brief 写文件
 */
int write(int fd, char* buf, int len);

/**
 * @brief 调整当前读写位置
 */
int lseek(int fd, int offset, int dir);

/**
 * @brief 关闭文件
 */
int close(int fd);

#endif // LIB_SYSCALL_H