/***
 * 
 * 系统调用相关
 * 
 */
#include "sys/syscall.h"
#include "core/task_t.h"
#include "tools/log.h"

static const 
syscall_handler_t sys_table[] = {
    [SYS_sleep]  = (syscall_handler_t)sys_sleep,
    [SYS_getpid] = (syscall_handler_t)sys_getpid,
    [SYS_fork]   = (syscall_handler_t)sys_fork,
    [SYS_execve] = (syscall_handler_t)sys_execve,
    [SYS_yield]  = (syscall_handler_t)sys_yield,
    [SYS_exit]   = (syscall_handler_t)sys_exit,
    [SYS_wait]   = (syscall_handler_t)sys_wait,
    
    [SYS_sbrk]   = (syscall_handler_t)sys_sbrk,

    [SYS_unlink] = (syscall_handler_t)sys_unlink,
    [SYS_open]   = (syscall_handler_t)sys_open,
    [SYS_read]   = (syscall_handler_t)sys_read,
    [SYS_write]  = (syscall_handler_t)sys_write,
    [SYS_lseek]  = (syscall_handler_t)sys_lseek,
    [SYS_close]  = (syscall_handler_t)sys_close,
    [SYS_isatty] = (syscall_handler_t)sys_isatty,
    [SYS_fstat]  = (syscall_handler_t)sys_fstat,
    [SYS_dup]    = (syscall_handler_t)sys_dup,
    [SYS_ioctl]  = (syscall_handler_t)sys_ioctl,

    [SYS_opendir]  = (syscall_handler_t)sys_opendir,
    [SYS_readdir]  = (syscall_handler_t)sys_readdir,
    [SYS_closedir] = (syscall_handler_t)sys_closedir,

    [SYS_gettimeofday] = (syscall_handler_t)sys_gettimeofday,
};

void do_handler_syscall(exception_frame_t* frame) {
    task_t* task = get_curr_task();

    int id = frame->eax;
    int arg0 = frame->ebx;
    int arg1 = frame->ecx;
    int arg2 = frame->edx;
    int arg3 = frame->esi;
    int arg4 = frame->edi;

    if(id >= sizeof(sys_table) / sizeof(sys_table[0])) {
        goto syscall_id_error;
    }

    syscall_handler_t handler = sys_table[id];
    if(handler == NULL) {
        frame->eax = -1;
        goto syscall_handler_null;
    }

    int ret = handler(arg0, arg1, arg2, arg3, arg4);
    frame->eax = ret;
    return;

syscall_id_error:
    log_print("task: %s, Unknown syscall: %d.", task->name, id);
    return;
syscall_handler_null:
    log_print("task: %s, Syscall Handler Null.", task->name);
    return;
}