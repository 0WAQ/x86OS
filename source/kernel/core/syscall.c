/***
 * 
 * 系统调用相关
 * 
 */
#include "core/syscall.h"
#include "core/task.h"
#include "tools/log.h"

static const 
syscall_handler_t sys_table[] = {
    [SYS_sleep] = (syscall_handler_t)sys_sleep,
};

void do_handler_syscall(syscall_frame_t* frame) {

    task_t* task = get_curr_task();

    if(frame->id >= sizeof(sys_table) / sizeof(sys_table[0])) {
        goto syscall_id_error;
    }

    syscall_handler_t handler = sys_table[frame->id];
    if(!handler) {
        goto syscall_handler_null;
    }

    int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
    return;


syscall_id_error:
    log_print("task: %s, Unknown syscall: %d.", task->name, frame->id);
    return;
syscall_handler_null:
    log_print("task: %s, Syscall Handler Null.", task->name);
    return;
}