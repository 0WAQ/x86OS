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

    // 编译器会将 -2147483648视为ll，压栈时会将其解析为8字节，先压入4字节的0xffffffff，
    //  再压入4字节的0x80000000
    // 导致在解析args时，左边的%d(在vsprintf中，以int提取)，只会提取低4字节的0x80000000，
    //  而高4字节的0xffffffff会留在栈中，对之后参数的提取造成影响

    // 例如，压入以下参数时，从低字节到高字节分别为(参数从右到左入栈)：
    // 0x00000061(a), 0x80000000, 0xffffffff, 0x00000062(b)
    // 导致在本该提取b的va_arg，提取了0xffffffff, 也即-1

    // 解决方法：将其强转为int，使其在压栈时被视为4字节数
    log_print("%c %d %d", 'a', -2147483648, 'b');
    // 为什么会将-2147483648视为ll？

    for(;;);
}