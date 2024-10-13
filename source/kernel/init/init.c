/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "cpu/cpu.h"

void kernel_init(boot_info_t* boot_info) {
    
    // 初始化 cpu
    cpu_init();

    for(;;);
}