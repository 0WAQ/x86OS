/***
 * 
 * 系统调用的外部相关头文件
 * 
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "lib_syscall_t.h"
#include "fs/fs_t.h"
#include <sys/stat.h>
#include <sys/time.h>

/**
 * @brief 系统调用的统一入口
 */
static inline
int syscall(int id, syscall_args_t * args);

/**
 * @brief 让当前进程进入sleep
 */
int msleep(int ms);

/**
 * @brief 获取当前进程的pid
 */
int getpid();

/**
 * @brief 创建子进程
 */
int fork();

/**
 * @brief 从文件中执行新进程
 */
int execve(const char* path, char* const* argv, char* const* env);

/**
 * @brief  TODO:
 */
int kill(int pid, int sig);

/**
 * @brief 让进程主动释放cpu
 */
int yield();

/**
 * @brief 删除文件
 */
int unlink(const char* filename);

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

/**
 * @brief TODO: 
 */
int isatty(int fd);

/**
 * @brief
 */
int fstat(int fd, struct stat* st);

/**
 * @brief io控制系统调用的用户接口
 */
int ioctl(int fd, int cmd, int arg0, int arg1);

/**
 * @brief 动态分配堆内存
 */
void* sbrk (ptrdiff_t incr);

/**
 * @brief 为进程创建一个空闲的文件描述符分配给fd对应的文件
 */
int dup(int fd);

/**
 * @brief sys_exit的外部实现
 */
void _exit(int status);

/**
 * @brief sys_wait的外部实现
 */
int wait(int* status);

/**
 * @brief 打开目录的外部实现
 */
#ifndef FIRST_TASK
DIR* opendir(const char* path);
#endif // FIRST_TASK

/**
 * @brief 读取目录的外部实现
 */
#ifndef FIRST_TASK
struct dirent* readdir(DIR* dir);
#endif // FIRST_TASK

/**
 * @brief 关闭目录的外部实现
 */
#ifndef FIRST_TASK
int closedir(DIR* dir);
#endif // FIRST_TASK

/**
 * @brief
 */
int gettimeofday(struct timeval* tv, void* tz);

#endif // LIB_SYSCALL_H