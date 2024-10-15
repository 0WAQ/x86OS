/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"

void kernel_init(boot_info_t* boot_info) {    
    // 初始化cpu，并重新加载gdt
    cpu_init();

    // 初始化idt
    irq_init();

    // 该函数返回后，会长跳转至gdt_reload，并跳转到init
}

void init() {
    for(;;);
}