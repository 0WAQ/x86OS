#include "sys/syscall.h"

void sys_sleep(u32_t ms) {
    
    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection(); 
    
    task_t* curr = get_curr_task();
    set_task_block(curr);
    set_task_sleep(curr, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);

    task_dispatch();

    irq_leave_protection(state);
    /////////////////////////////////////////// 退出临界区

}