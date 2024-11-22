/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "common/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "ipc/sem.h"
#include "dev/time.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "core/task.h"
#include "core/memory.h"
#include "os_cfg.h"

// 该函数返回后，会长跳转至gdt_reload，并跳转到kernel_main
void kernel_init(boot_info_t* boot_info) {

    ASSERT(boot_info->ram_region_count != 0);

    // 初始化cpu
    cpu_init();

    // 初始化idt
    irq_init();

    // 初始化日志
    log_init();

    // 初始化内存
    memroy_init(boot_info);

    // 初始化定时器
    time_init();

    // 初始化任务管理器
    task_manager_init();
}

void kernel_main() {

    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);

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