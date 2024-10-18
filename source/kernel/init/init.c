/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "os_cfg.h"

void kernel_init(boot_info_t* boot_info) {    
    // 初始化cpu，并重新加载gdt
    cpu_init();

    // 初始化日志
    log_init();

    // 初始化idt
    irq_init();

    // 初始化定时器
    time_init();

    // 该函数返回后，会长跳转至gdt_reload，并跳转到init
}

void init_main() {
    // irq_enalbe_global();
    
    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);
    
    for(;;);
}