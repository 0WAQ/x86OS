#include "sys/syscall.h"

// TODO: 发现 bug, 当在 shell 中递归使用 shell 后进行退出时, 会 page fault
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

    // 判断是否需要将子进程交给 1 号进程管理
    bool move_child = false;

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
                    move_child = true;
                }
            }
        }
    }
    mutex_unlock(&task_table_mutex);
    ////////////////////////////////////////////////
    
    irq_state_t state = irq_enter_protection();

    task_t* parent = curr_task->parent;

    // 当前进程的子进程由 1 号进程回收
    if(move_child && (parent != task_manager.first_task)) {
        if(task_manager.first_task->state == TASK_WAITING) {
            task_set_ready(task_manager.first_task);
        }
    }
    if(parent->state == TASK_WAITING) {
        task_set_ready(parent);
    }

    curr_task->state = TASK_ZOMBIE;
    curr_task->status = status;      // 保存进程退出时的状态
    task_dispatch();

    irq_leave_protection(state);
}