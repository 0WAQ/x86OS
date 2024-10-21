/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "common/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "core/task.h"
#include "os_cfg.h"

// 该函数返回后，会长跳转至gdt_reload，并跳转到init_main
void kernel_init(boot_info_t* boot_info) {

    ASSERT(boot_info->ram_region_count != 0);

    // 初始化cpu，并重新加载gdt
    cpu_init();

    // 初始化日志
    log_init();

    // 初始化idt
    irq_init();

    // 初始化定时器
    time_init();
}

static task_t task1, task2;
static uint32_t task2_stack[1024];

void task2_entry() {
    int cnt = 0;
    for(;;) {
        log_print("task2: %d", cnt++);
        task_switch_from_to(&task2, &task1);
    }
}

void init_main() {
    // irq_enalbe_global();

    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);

    // 这里不需要给参数，因为当cpu从当前任务切换走时，会将状态保存，只要保证有地方可去就行
    task_init(&task1, (uint32_t)0, 0);
    task_init(&task2, (uint32_t)task2_entry, (uint32_t)&task2_stack[1024]);

    // tr寄存器默认为空，加载task1到tr
    // ltr(task1.tss_sel);  // 若使用task_switch_from_to使用switch_tss，那么需要启用改行

    int cnt = 0;
    for(;;) {
        log_print("task1: %d", cnt++);
        task_switch_from_to(&task1, &task2);
    }

    task2_entry();
}