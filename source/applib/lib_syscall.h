/***
 * 系统调用对外接口
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "applib/lib_syscall_t.h"

#include <sys/stat.h>
#include <sys/time.h>

/**
 * @brief 系统调用的统一入口
 */
static inline
int syscall(int id, syscall_args_t * args);


int msleep(int ms);
int getpid();
int fork();
int execve(const char* path, char* const* argv, char* const* env);
int kill(int pid, int sig); // TODO:
int yield();

int unlink(const char* filename);
int open(const char* filename, int flags, ...);
int read(int fd, char* buf, int len);
int write(int fd, char* buf, int len);
int lseek(int fd, int offset, int dir);
int close(int fd);

int isatty(int fd); // TODO:
int fstat(int fd, struct stat* st);
int ioctl(int fd, int cmd, int arg0, int arg1);
void* sbrk (ptrdiff_t incr);
int dup(int fd);

void _exit(int status);
int wait(int* status);

#ifndef FIRST_TASK

#include "fs/fs_t.h"

DIR* opendir(const char* path);
struct dirent* readdir(DIR* dir);
int closedir(DIR* dir);

#endif // FIRST_TASK

int gettimeofday(struct timeval* tv, void* tz);

#endif // LIB_SYSCALL_H