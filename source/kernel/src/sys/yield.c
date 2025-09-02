#include "sys/syscall.h"

int sys_yield() {
    irq_state_t state = irq_enter_protection();

    // 判断就绪队列中是否还有任务
    if (task_manager.cfs_rq.nr > 1) {
        task_dispatch();
    }
    irq_leave_protection(state);
    return 0;
}