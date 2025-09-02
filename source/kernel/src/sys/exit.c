#include "sys/syscall.h"

void sys_exit(int status) {

    // 清空任务相关的资源
    task_t* curr_task = get_curr_task();

    // 清空任务打开的文件
    for(int fd = 0; fd < TASK_OFILE_NR; fd++) {
        file_t* file = curr_task->file_table[fd];
        if(file) {
            sys_close(fd);
            curr_task->file_table[fd] = NULL;
        }
    }

    int move_child = 0;

    ////////////////////////////////////////////////
    mutex_lock(&task_table_mutex);
    {
        // 一个进程退出时, 将其所有的子进程交给一号进程
        for(int i = 0; i < TASK_NR; i++) {
            task_t* p = task_table + i;
            if(p->parent == curr_task) {
                p->parent = task_manager.first_task;

                // 若有子进程是 zombie 态
                if(p->state == TASK_ZOMBIE) {
                    move_child = 1;
                }
            }
        }
    }
    mutex_unlock(&task_table_mutex);
    ////////////////////////////////////////////////
    
    irq_state_t state = irq_enter_protection();

    task_t* parent = curr_task->parent;
    if(move_child && (parent != task_manager.first_task)) {
        if(task_manager.first_task->state == TASK_WAITTING) {
            task_set_ready(task_manager.first_task);
        }
    }
    if(parent->state == TASK_WAITTING) {
        task_set_ready(parent);
    }

    task_set_block(curr_task);
    curr_task->state = TASK_ZOMBIE;
    curr_task->status = status;      // 保存进程退出时的状态
    task_dispatch();

    irq_leave_protection(state);
}