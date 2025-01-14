/***
 * 
 * 系统调用相关
 * 
 */
#include "core/syscall.h"
#include "core/task.h"
#include "core/memory.h"
#include "fs/fs.h"
#include "tools/log.h"

// 临时 TODO:
void sys_print(char* fmt, int arg) {
    log_print(fmt, arg);
}

static const 
syscall_handler_t sys_table[] = {
    [SYS_sleep]  = (syscall_handler_t)sys_sleep,
    [SYS_getpid] = (syscall_handler_t)sys_getpid,
    [SYS_fork]   = (syscall_handler_t)sys_fork,
    [SYS_execve] = (syscall_handler_t)sys_execve,
    [SYS_yield]  = (syscall_handler_t)sys_yield,
    [SYS_exit]   = (syscall_handler_t)sys_exit,
    [SYS_wait]   = (syscall_handler_t)sys_wait,
    
    [SYS_open]   = (syscall_handler_t)sys_open,
    [SYS_read]   = (syscall_handler_t)sys_read,
    [SYS_write]  = (syscall_handler_t)sys_write,
    [SYS_lseek]  = (syscall_handler_t)sys_lseek,
    [SYS_close]  = (syscall_handler_t)sys_close,
    [SYS_isatty] = (syscall_handler_t)sys_isatty,
    [SYS_fstat]  = (syscall_handler_t)sys_fstat,
    [SYS_sbrk]   = (syscall_handler_t)sys_sbrk,
    [SYS_dup]    = (syscall_handler_t)sys_dup,
    
    [SYS_opendir]  = (syscall_handler_t)sys_opendir,
    [SYS_readdir]  = (syscall_handler_t)sys_readdir,
    [SYS_closedir] = (syscall_handler_t)sys_closedir,

    [SYS_print]  = (syscall_handler_t)sys_print,
};

void do_handler_syscall(syscall_frame_t* frame) {

    task_t* task = get_curr_task();
    frame->eax = -1;

    if(frame->id >= sizeof(sys_table) / sizeof(sys_table[0])) {
        goto syscall_id_error;
    }

    syscall_handler_t handler = sys_table[frame->id];
    if(!handler) {
        goto syscall_handler_null;
    }

    int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
    frame->eax = ret;
    return;


syscall_id_error:
    log_print("task: %s, Unknown syscall: %d.", task->name, frame->id);
    return;
syscall_handler_null:
    log_print("task: %s, Syscall Handler Null.", task->name);
    return;
}