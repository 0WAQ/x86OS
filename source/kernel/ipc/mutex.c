/***
 * 
 * 互斥锁实现
 * 
 */
#include "ipc/mutex.h"
#include "cpu/irq.h"

void mutex_init(mutex_t* mutex) {
    mutex->owner = NULL;
    mutex->count = 0;
    list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t* mutex) {
    uint32_t state = irq_enter_protection();

    task_t* curr = get_curr_task();

    // 锁是空闲的
    if(mutex->count == 0) {
        ++mutex->count;
        mutex->owner = curr;
    }
    // 若锁被同一任务拥有, 那就无需等待, 继续运行
    else if(mutex->owner == curr) {
        ++mutex->count;
    }
    // 该锁被其它任务拥有, 阻塞, 将其放到等待队列
    else {
        set_task_block(curr);
        list_insert_last(&mutex->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protectoin(state);
}

void mutex_unlock(mutex_t* mutex) {
    uint32_t state = irq_enter_protection();

    task_t* curr = get_curr_task();

    // 只有锁的拥有者才可以执行解锁
    if(mutex->owner == curr) {
        // 引用计数为0时才可以真正解锁
        if(--mutex->count == 0) {
            mutex->owner = NULL;
        
            // 然后让其它竞争锁的任务执行(如果有)
            if(list_count(&mutex->wait_list)) {
                // 取出一个任务
                list_node_t* node = list_remove_first(&mutex->wait_list);
                task_t* task = list_entry_of(node, task_t, wait_node);

                // 加到就绪队列中
                set_task_ready(task);

                // 修改锁的状态
                mutex->count = 1;
                mutex->owner = task;
            
                task_dispatch();
            }
        }
    }

    irq_leave_protectoin(state);
}