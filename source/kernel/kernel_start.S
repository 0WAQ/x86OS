
# gcc汇编器默认代码是32位

#include "common/cfg.h"

    .text
    .extern kernel_init
    .extern kernel_main
    .global _start      # void _start(boot_info_t* boot_info)

# 0x10000
_start:
    # 将 [esp + 0x4] 即 boot_info 的地址 0xa000 压入栈中
    push 0x4(%esp)

    # kernel_init(boot_info_t*)
    call kernel_init

    # 长跳转, 用以刷新段选择子
    jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
    mov $(stack + KERNEL_STACK_SIZE), %esp
    jmp kernel_main

# 设置内核栈
    .bss
.comm stack, KERNEL_STACK_SIZE
