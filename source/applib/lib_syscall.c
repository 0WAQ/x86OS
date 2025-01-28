/**
 * 
 * 系统调用接口
 * 
 */
#include "lib_syscall.h"
#include "core/syscall_t.h"
#include "os_cfg.h"

#ifndef FIRST_TASK
#include <stdlib.h>
#endif // FIRST_TASK

// 通过中断实现系统调用
static inline
int syscall(int id, syscall_args_t* args) {
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        :"=a"(ret)          // 返回值通过eax寄存器传递给ret
        :"D"(args->arg4), "S"(args->arg3), "d"(args->arg2),
         "c"(args->arg1), "b"(args->arg0), "a"(id)
    );
    return ret;
}

int msleep(int ms) {
    if(ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.arg0 = ms;
    return syscall(SYS_sleep, &args);
}

int getpid() {
    syscall_args_t args;
    return syscall(SYS_getpid, &args);
}

int fork() {
    syscall_args_t args;
    return syscall(SYS_fork, &args);
}

int execve(const char* path, char* const* argv, char* const* env) {
    syscall_args_t args;
    args.arg0 = (int)path;
    args.arg1 = (int)argv;
    args.arg2 = (int)env;
    return syscall(SYS_execve, &args);
}

int kill(int pid, int sig) {
    return -1;
}

int yield() {
    syscall_args_t args;
    return syscall(SYS_yield, &args); 
}

void _exit(int status) {
    syscall_args_t args;
    args.arg0 = status;
    syscall(SYS_exit, &args);
    __builtin_unreachable();
}

int wait(int* status) {
    syscall_args_t args;
    args.arg0 = (int)status;
    return syscall(SYS_wait, &args);
}

int unlink(const char* filename) {
    syscall_args_t args;
    args.arg0 = (int)filename;
    return syscall(SYS_unlink, &args);
}

int open(const char* filename, int flags, ...) {
    syscall_args_t args;
    args.arg0 = (int)filename;
    args.arg1 = flags;
    return syscall(SYS_open, &args);
}

int read(int fd, char* buf, int len) {
    syscall_args_t args;
    args.arg0 = fd;
    args.arg1 = (int)buf;
    args.arg2 = len;
    return syscall(SYS_read, &args);
}

int write(int fd, char* buf, int len) {
    syscall_args_t args;
    args.arg0 = fd;
    args.arg1 = (int)buf;
    args.arg2 = len;
    return syscall(SYS_write, &args);
}

int lseek(int fd, int offset, int dir) {
    syscall_args_t args;
    args.arg0 = fd;
    args.arg1 = offset;
    args.arg2 = dir;
    return syscall(SYS_lseek, &args);
}

int close(int fd) {
    syscall_args_t args;
    args.arg0 = fd;
    return syscall(SYS_close, &args);
}

int isatty(int fd) {
    syscall_args_t args;
    args.arg0 = fd;
    return syscall(SYS_isatty, &args);
}

int fstat(int fd, struct stat* st) {
    syscall_args_t args;
    args.arg0 = fd;
    args.arg1 = (int)st;
    return syscall(SYS_fstat, &args);
}

int ioctl(int fd, int cmd, int arg0, int arg1) {
    syscall_args_t args;
    args.arg0 = fd;
    args.arg1 = (int)cmd;
    args.arg2 = arg0;
    args.arg3 = arg1;
    return syscall(SYS_ioctl, &args);
}

void* sbrk (ptrdiff_t incr) {
    syscall_args_t args;
    args.arg0 = (int)incr;
    return (void*)syscall(SYS_sbrk, &args);
}

int dup(int fd) {
    syscall_args_t args;
    args.arg0 = (int)fd;
    return syscall(SYS_dup, &args);
}

#ifndef FIRST_TASK
DIR* opendir(const char* path) {
    DIR* dir = (DIR*)malloc(sizeof(DIR));
    if(dir == NULL) {
        return NULL;
    }

    syscall_args_t args;
    args.arg0 = (int)path;
    args.arg1 = (int)dir;
    int ret = syscall(SYS_opendir, &args);
    if(ret < 0) {
        free(dir);
        return NULL;
    }

    return dir;
}
#endif // FIRST_TASK

#ifndef FIRST_TASK
struct dirent* readdir(DIR* dir) {
    syscall_args_t args;
    args.arg0 = (int)dir;
    args.arg1 = (int)&dir->dirent;
    int ret = syscall(SYS_readdir, &args);
    if(ret < 0) {
        return NULL;
    }
    return &dir->dirent;
}
#endif // FIRST_TASK

#ifndef FIRST_TASK
int closedir(DIR* dir) {
    syscall_args_t args;
    args.arg0 = (int)dir;
    int ret = syscall(SYS_closedir, &args);
    free(dir);
    return 0;
}
#endif // FIRST_TASK

int gettimeofday(struct timeval* tv, void* tz) {
    syscall_args_t args;
    args.arg0 = (int)tv;
    args.arg1 = (int)tz;
    return syscall(SYS_gettimeofday, &args);
}