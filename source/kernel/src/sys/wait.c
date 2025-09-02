#include "sys/syscall.h"

int sys_wait(int* status) {
    task_t* task = get_curr_task();
    while(1) {
        // 一直寻找处于 zombie 态的子进程, 若找不到则 sleep
        mutex_lock(&task_table_mutex);
        for (int i = 0; i < TASK_NR; i++) {
            task_t* p = task_table + i;
            if(p->parent != task) {
                continue;
            }

            // 若子进程是 zombie 态
            if(p->state == TASK_ZOMBIE) {
                int pid = p->pid;
                *status = p->status;

                vmem_destory_uvms(p->tss.cr3);
                vmem_free_page(p->tss.esp0 - MEM_PAGE_SIZE);
                kmemset(p, 0, sizeof(task_t));
            
                mutex_unlock(&task_table_mutex);
                return pid;
            }
        }
        mutex_unlock(&task_table_mutex);

        // 找不到则等待, 知道有子进程调用 exit
        irq_state_t state = irq_enter_protection();
        task_set_block(task);
        task->state = TASK_WAITTING;
        task_dispatch();
        irq_leave_protection(state);
    }

    return 0;
}