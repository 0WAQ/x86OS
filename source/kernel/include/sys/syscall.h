/**
 * 系统调用内核接口
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "sys/syscall_t.h"

#include "core/task.h"
#include "core/memory.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "cpu/mmu.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "dev/disk.h"
#include "dev/kbd.h"
#include "dev/timer.h"
#include "dev/tty.h"
#include "fs/fs.h"
#include "fs/file.h"
#include "fs/devfs/devfs.h"
#include "fs/fat16fs/fat16fs.h"
#include "ipc/mutex.h"
#include "ipc/sem.h"
#include "tools/bitmap.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "tools/log.h"
#include "tools/rbtree.h"
#include "tools/time.h"
#include "common/cfg.h"

#include <sys/time.h>
#include <fcntl.h>

extern task_manager_t task_manager;
extern task_t task_table[TASK_NR];
extern mutex_t task_table_mutex;

extern fs_t* root_fs;
extern list_t mounted_list;

void exception_handler_syscall();
void do_handler_syscall(exception_frame_t* frame);

////////////////////////////////////////////////////////////////////

void sys_sleep(u32_t ms);
int sys_getpid();
int sys_fork();
int sys_execve(char* path, char** argv, char** env);
int sys_yield();
void sys_exit(int status);
int sys_wait(int* status);
char* sys_sbrk(int incr);

int sys_unlink(const char* filename);
int sys_open(const char* filename, int flags, ...);
int sys_read(int fd, char* buf, int len);
int sys_write(int fd, char* buf, int len);
int sys_lseek(int fd, int offset, int dir);
int sys_close(int fd);

int sys_isatty(int fd);
int sys_fstat(int fd, struct stat* st);
int sys_ioctl(int fd, int cmd, int arg0, int arg1);
int sys_dup(int fd);

int sys_opendir(const char* path, DIR* dir);
int sys_readdir(DIR* dir, struct dirent* dirent);
int sys_closedir(DIR* dir);

int sys_gettimeofday(struct timeval* tv, struct timezone* tz);

#endif // SYSCALL_H

////////////////////////////////////////////////////////////////////

#ifdef SYSCALL_T_H
#endif
#ifdef TASK_H
#endif
#ifdef MEMORY_H
#endif
#ifdef CPU_H
#endif
#ifdef IRQ_H
#endif
#ifdef MMU_H
#endif
#ifdef CONSOLE_H
#endif
#ifdef DEV_H
#endif
#ifdef DISK_H
#endif
#ifdef KBD_H
#endif
#ifdef TIMER_H
#endif
#ifdef TTY_H
#endif
#ifdef FS_H
#endif
#ifdef FILE_H
#endif
#ifdef DEVFS_H
#endif
#ifdef FAT16FS_H
#endif
#ifdef MUTEX_H
#endif
#ifdef SEM_H
#endif
#ifdef BITMAP_H
#endif
#ifdef KLIB_H
#endif
#ifdef LIST_H
#endif
#ifdef LOG_H
#endif
#ifdef RBTREE_H
#endif
#ifdef TIME_H
#endif