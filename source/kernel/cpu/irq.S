/**
 *
 * irq中各种异常的宏
 *
 */

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

# 异常处理函数的生成宏
exception_handler unknown, -1, 0
exception_handler divider, 0, 0
exception_handler debug, 1, 0
exception_handler NMI, 2, 0
exception_handler breakpoint, 3, 0
exception_handler overflow, 4, 0
exception_handler bound_range, 5, 0
exception_handler invalid_opcode, 6, 0
exception_handler device_unavailable, 7, 0
exception_handler double_fault, 8, 1
exception_handler invalid_tss, 10, 1
exception_handler segment_not_present, 11, 1
exception_handler stack_segment_fault, 12, 1
exception_handler general_protection, 13, 1
exception_handler page_fault, 14, 1
exception_handler fpu_error, 16, 0
exception_handler alignment_check, 17, 1
exception_handler machine_check, 18, 0
exception_handler smd_exception, 19, 0
exception_handler virtual_exception, 20, 0

# 定时器的中断处理函数
exception_handler timer, 0x20, 0

# 系统调用的处理函数
    .global exception_handler_syscall
    .extern do_handler_syscall
exception_handler_syscall:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    pushf

    mov %esp, %eax
    push %eax
    call do_handler_syscall
    add $4, %esp

    popf
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa

    retf $(5 * 4)
