/**
 * 
 * 系统调用相关头文件
 * 
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "syscall_t.h"

#include "cpu/irq.h"
#include "core/task.h"
#include "core/memory.h"
#include "fs/fs.h"

#include "tools/log.h"
#include "tools/klib.h"
#include "tools/time.h"
#include "os_cfg.h"

#include <sys/time.h>
#include <fcntl.h>

extern task_manager_t task_manager;
extern task_t task_table[TASK_NR];
extern mutex_t task_table_mutex;

extern fs_t* root_fs;
extern list_t mounted_list;

void exception_handler_syscall();
void do_handler_syscall(exception_frame_t* frame);

/**
 * @brief 让任务睡眠
 */
void sys_sleep(u32_t ms);

/**
 * @brief 获取任务的pid
 */
int sys_getpid();

/**
 * @brief 创建子进程
 */
int sys_fork();

/**
 * @brief execve系统调用, 会涉及到不同进程空间中数据的传递
 */
int sys_execve(char* path, char** argv, char** env);

/**
 * @brief 会让任务主动放弃cpu
 */
int sys_yield();

/**
 * @brief exit系统调用的内部实现
 */
void sys_exit(int status);

/**
 * @brief 实现wait系统调用
 */
int sys_wait(int* status);

/**
 * @brief 控制堆的增长
 * @param incr increment, 堆增长的字节
 * @return 成功返回新分配空间的起始地址(原堆的结束地址), 失败返回-1
 */
char* sys_sbrk(int incr);

/**
 * @brief 系统调用: 删除文件(减少引用计数)
 */
int sys_unlink(const char* filename);

/**
 * @brief 系统调用: 打开一个文件
 */
int sys_open(const char* filename, int flags, ...);

/**
 * @brief 系统调用: 从fd对应的文件中读取
 */
int sys_read(int fd, char* buf, int len);

/**
 * @brief 系统调用: 向fd对应的文件写
 */
int sys_write(int fd, char* buf, int len);

/**
 * @brief 系统调用: 定位fd的读取写入位置
 */
int sys_lseek(int fd, int offset, int dir);

/**
 * @brief 系统调用: 关闭fd
 */
int sys_close(int fd);

/**
 * @brief 系统调用: 判断fd是否是一个tty设备
 */
int sys_isatty(int fd);

/**
 * @brief 系统调用: 返回文件状态信息
 */
struct stat;
int sys_fstat(int fd, struct stat* st);

/**
 * @brief 系统调用: 对输入输出设备进行控制 TODO: 将其改为可变参传递(前提: 将系统调用改为可变参传递)
 */
int sys_ioctl(int fd, int cmd, int arg0, int arg1);

/**
 * @brief 系统调用: 拷贝(增加引用计数)fd对应的file
 */
int sys_dup(int fd);

/**
 * @brief 系统调用: 打开目录
 */
int sys_opendir(const char* path, DIR* dir);

/**
 * @brief 系统调用: 读取目录
 */
int sys_readdir(DIR* dir, struct dirent* dirent);

/**
 * @brief 系统调用: 关闭目录
 */
int sys_closedir(DIR* dir);

/**
 * @brief 系统调用: 
 */
int sys_gettimeofday(struct timeval* tv, struct timezone* tz);

#endif // SYSCALL_H