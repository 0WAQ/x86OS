#include "sys/syscall.h"

void sys_exit(int status) {

    // 清空任务相关的资源
    task_t* task = get_curr_task();
    for(int fd = 0; fd < TASK_OFILE_NR; fd++) {
        file_t* file = task->file_table[fd];
        if(file) {
            sys_close(fd);
            task->file_table[fd] = NULL;
        }
    }

    int move_child = 0;

    ////////////////////////////////////////////////
    mutex_lock(&task_table_mutex);
    {
        // 一个进程退出时, 将其所有的子进程交给一号进程
        for(int i = 0; i < TASK_NR; i++) {
            task_t* p = task_table + i;
            if(p->parent == task) {
                p->parent = task_manager.first_task;

                // 若有子进程是zombie态
                if(p->state == TASK_ZOMBIE) {
                    move_child = 1;
                }
            }
        }
    }
    mutex_unlock(&task_table_mutex);
    ////////////////////////////////////////////////

    task->state = TASK_ZOMBIE;
    task->status = status;      // 保存进程退出时的状态

    irq_state_t state = irq_enter_protection();

    task_t* parent = task->parent;
    if(move_child && (parent != task_manager.first_task)) {
        if(task_manager.first_task->state == TASK_WAITTING) {
            set_task_ready(task_manager.first_task);
        }
    }

    if(parent->state == TASK_WAITTING) {
        set_task_ready(parent);
    }

    set_task_block(task);
    task_dispatch();

    irq_leave_protection(state);
}