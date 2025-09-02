#include "sys/syscall.h"

void sys_sleep(u32_t ms) {
    irq_state_t state = irq_enter_protection(); 

    task_t* curr = get_curr_task();
    task_set_sleep(curr, (ms + (OS_TICK_MS - 1)) / OS_TICK_MS);
    task_dispatch();

    irq_leave_protection(state);
}