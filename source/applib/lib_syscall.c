/**
 * 
 * 系统调用接口
 * 
 */
#include "lib_syscall.h"
#include "core/syscall.h"
#include "os_cfg.h"
#include <stdlib.h>

static inline
int sys_call(syscall_args_t * args) {
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

int msleep(int ms) {
    if(ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_sleep;
    args.arg0 = ms;

    return sys_call(&args);
}

int getpid() {
    syscall_args_t args;
    args.id = SYS_getpid;
    return sys_call(&args);
}

int print(const char* fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_print;
    args.arg0 = (uint32_t)fmt;
    args.arg1 = arg;
    return sys_call(&args);
}

int fork() {
    syscall_args_t args;
    args.id = SYS_fork;
    return sys_call(&args);
}

int execve(const char* path, char* const* argv, char* const* env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.arg0 = (int)path;
    args.arg1 = (int)argv;
    args.arg2 = (int)env;
    return sys_call(&args);
}

int yield() {
    syscall_args_t args;
    args.id = SYS_yield;
    return sys_call(&args); 
}

void _exit(int status) {
    syscall_args_t args;
    args.id = SYS_exit;
    args.arg0 = status;
    sys_call(&args);
    __builtin_unreachable();
}

int wait(int* status) {
    syscall_args_t args;
    args.id = SYS_wait;
    args.arg0 = (int)status;
    return sys_call(&args);
}

int open(const char* filename, int flags, ...) {
    syscall_args_t args;
    args.id = SYS_open;
    args.arg0 = (int)filename;
    args.arg1 = flags;
    return sys_call(&args);
}

int read(int fd, char* buf, int len) {
    syscall_args_t args;
    args.id = SYS_read;
    args.arg0 = fd;
    args.arg1 = (int)buf;
    args.arg2 = len;
    return sys_call(&args);
}

int write(int fd, char* buf, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.arg0 = fd;
    args.arg1 = (int)buf;
    args.arg2 = len;
    return sys_call(&args);
}

int lseek(int fd, int offset, int dir) {
    syscall_args_t args;
    args.id = SYS_lseek;
    args.arg0 = fd;
    args.arg1 = offset;
    args.arg2 = dir;
    return sys_call(&args);
}

int close(int fd) {
    syscall_args_t args;
    args.id = SYS_close;
    args.arg0 = fd;
    return sys_call(&args);
}

int isatty(int fd) {
    syscall_args_t args;
    args.id = SYS_isatty;
    args.arg0 = fd;
    return sys_call(&args);
}

int fstat(int fd, struct stat* st) {
    syscall_args_t args;
    args.id = SYS_fstat;
    args.arg0 = fd;
    args.arg1 = (int)st;
    return sys_call(&args);
}

void* sbrk (ptrdiff_t incr) {
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.arg0 = (int)incr;
    return (void*)sys_call(&args);
}

int dup(int fd) {
    syscall_args_t args;
    args.id = SYS_dup;
    args.arg0 = (int)fd;
    return sys_call(&args);
}

DIR* opendir(const char* path) {
    DIR* dir = (DIR*)malloc(sizeof(DIR));
    if(dir == NULL) {
        return NULL;
    }

    syscall_args_t args;
    args.id = SYS_opendir;
    args.arg0 = (int)path;
    args.arg1 = (int)dir;
    int ret = sys_call(&args);
    if(ret < 0) {
        free(dir);
        return NULL;
    }

    return dir;
}

struct dirent* readdir(DIR* dir) {
    syscall_args_t args;
    args.id = SYS_readdir;
    args.arg0 = (int)dir;
    args.arg1 = (int)&dir->dirent;
    int ret = sys_call(&args);
    if(ret < 0) {
        return NULL;
    }
    return &dir->dirent;
}

int closedir(DIR* dir) {
    syscall_args_t args;
    args.id = SYS_closedir;
    args.arg0 = (int)dir;
    int ret = sys_call(&args);
    free(dir);
    return 0;
}
