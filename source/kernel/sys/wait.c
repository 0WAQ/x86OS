#include "sys/syscall.h"

int sys_wait(int* status) {
    task_t* task = get_curr_task();

    // 循环
    while(1) {

        // 一直寻找处于zombie态的子进程, 若找不到则sleep
        mutex_lock(&task_table_mutex);
        for(int i = 0; i < TASK_NR; i++) {
            task_t* p = task_table + i;
            if(p->parent != task) {
                continue;
            }

            // 若子进程是zombie态
            if(p->state == TASK_ZOMBIE) {
                int pid = p->pid;
                *status = p->status;

                memory_destory_uvm(p->tss.cr3);
                memory_free_page(p->tss.esp0 - MEM_PAGE_SIZE);
                kernel_memset(p, 0, sizeof(task_t));
            
                mutex_unlock(&task_table_mutex);
                return pid;
            }
        }
        mutex_unlock(&task_table_mutex);

        // 找不到就让父进程睡眠, 直到有子进程唤醒
        irq_state_t state = irq_enter_protection();
        set_task_block(task);
        task->state = TASK_WAITTING;

        // 切换到其它进程
        task_dispatch();

        irq_leave_protection(state);
    }

    return 0;
}