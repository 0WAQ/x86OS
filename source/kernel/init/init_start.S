
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


    .text
.macro exception_handler name num with_err_code
        .extern do_handler_\name
        .global exception_handler_\name
    exception_handler_\name:

        # 压入0表示没有错误码，有些指令会自动压入错误码，有些不会
        .if \with_err_code == 0
            push $0
        .endif

        # 压入异常号
        push $\num

        # 保存所有寄存器
        pushal
        push %ds
        push %es
        push %fs
        push %gs 

        # 调用默认异常处理程序，C实现
        push %esp   # esp在执行此条语句之前指向gs，压栈后，这个值就是exception_frame_t结构体的起始地址
        call do_handler_\name

        add $(1 * 4), %esp      # 丢掉esp

        # 恢复寄存器
        pop %gs
        pop %fs
        pop %es
        pop %ds
        popal

        # 跳过压入的异常号和错误码
        add $(2 * 4), %esp  # cpu不会自动将errno弹出(即使errno是由cpu自动压入的)
        iret
.endm

# 未知异常
exception_handler unknown, -1, 0

# 除0异常
exception_handler divider, 0, 0