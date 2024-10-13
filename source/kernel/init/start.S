
# gcc汇编器默认代码是32位

#include "os_cfg.h"

    .text
    .extern kernel_init
    .extern init
    # void _start(boot_info_t* boot_info)
    .global _start
_start:
    # 第一种方法
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax
    # push %eax

    # 第二种方法
    # mov 4(%esp), %eax
    # push %eax

    # 第三种方法
    push 4(%esp)

    # kernel_init(boot_info)
    call kernel_init

    jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
    mov $(stack + KERNEL_STACK_SIZE), %esp
    jmp init

# 设置内核栈
    .bss
.comm stack, KERNEL_STACK_SIZE