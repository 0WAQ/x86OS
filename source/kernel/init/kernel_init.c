/**
 * 内核初始化
 */
#include "init/kernel_init.h"
#include "core/task/task.h"
#include "core/memory/pmem.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "fs/fs.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/cfg.h"

// 该函数返回后，会长跳转至gdt_reload，并跳转到kernel_main
void kernel_init(boot_info_t* boot_info) {
    ASSERT(boot_info->ram_region_count != 0);

    cpu_init();

    irq_init();

    log_init();

    memroy_init(boot_info);

    fs_init();

    timer_init();

    task_manager_init();
}

void kernel_main() {
    log_print("==========================");
    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);
    log_print("==========================");

    // 初始化第一个任务
    first_task_init();

    // 运行第一个任务
    move_to_first_task();
}

void move_to_first_task() {
    task_t* curr = get_curr_task();
    ASSERT(curr != 0);

    tss_t* tss = &(curr->tss);
    
    __asm__ __volatile__(
        "push %[ss]\n\t"
        "push %[esp]\n\t"
        "push %[eflags]\n\t"
        "push %[cs]\n\t"
        "push %[eip]\n\t"
        "iret"
        :
        :[ss]"r"(tss->ss), [esp]"r"(tss->esp), [eflags]"r"(tss->eflags), 
         [cs]"r"(tss->cs), [eip]"r"(tss->eip)
    );
}