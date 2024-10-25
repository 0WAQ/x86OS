/**
 * 
 * 信号量相关实现
 * 
 */

#include "ipc/sem.h"
#include "core/task.h"
#include "cpu/irq.h"

void sem_init(sem_t* sem, uint32_t count) {
    list_init(&sem->wait_list);
    sem->count = count;
}

void sem_wait(sem_t* sem) {
    
    irq_state_t state =  irq_enter_protection();

    // 若计数大于0, 则获取信号量
    if(sem->count > 0) {
        --sem->count;
    }
    else {  // 否则将任务从运行队列中删除, 加入等待队列
        task_t* curr = get_curr_task();
        set_task_block(curr);
        list_insert_last(&sem->wait_list, &curr->wait_node);
    
        // 任务调度
        task_dispatch();
    }

    irq_leave_protectoin(state);
}

void sem_notify(sem_t* sem) {
    
    irq_state_t state = irq_enter_protection();
    
    // 若等待队列中有任务, 则将第一个任务移指就绪队列
    if(list_count(&sem->wait_list) > 0) {
        list_node_t* node = list_remove_first(&sem->wait_list);
        task_t* task = list_entry_of(node, task_t, wait_node);
    
        set_task_ready(task);

        // 任务调度
        task_dispatch();
    }
    else {
        ++sem->count;
    }

    irq_leave_protectoin(state);
}

uint32_t sem_count(sem_t* sem) {

    irq_state_t state = irq_enter_protection();
    uint32_t count = sem->count;
    irq_leave_protectoin(state);

    return count;
}