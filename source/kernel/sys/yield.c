#include "sys/syscall.h"

int sys_yield() {

    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection();


    // 判断就绪队列中是否有任务
    if(list_count(&task_manager.ready_list) > 1) {
        task_t* curr_task = get_curr_task();
        
        // 若就绪队列中还有其它任务,则将当前任务移入队列尾部
        set_task_block(curr_task);  // 先删除
        set_task_ready(curr_task);  // 再尾插
    
        // 任务切换
        task_dispatch();
    }


    irq_leave_protection(state);
    /////////////////////////////////////////// 退出临界区

    return 0;
}