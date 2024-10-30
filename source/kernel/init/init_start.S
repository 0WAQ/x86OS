
# gcc汇编器默认代码是32位

#include "os_cfg.h"

    .text
    .extern kernel_init
    .extern kernel_main
    .global _start      # void _start(boot_info_t* boot_info)
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
    jmp kernel_main

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


    .text
    .global simple_switch
simple_switch:  # simple_switch(&from->stack, to->stack)
    mov 4(%esp), %eax   # 取&from->stack
    mov 8(%esp), %edx   # 取to->stack

    /**
     * 切换任务时，保存当前任务的状态
     *    1.无需保存：
     *       cs, ds, es, fs, gs(因为是平坦模型)
     *       eax, ecx, edx编译器会自动保存
     *       eip无需保存，已经在栈中
     *       esp需要手动设置为新任务的状态
     *    2.需要保存：
     *       esi, edi, ebp, ebx需要保存
     */

    # 保存当前任务的状态
    push %ebp
    push %ebx
    push %esi
    push %edi
    
    # 切换栈
    mov %esp, (%eax)    # from->stack = esp
    mov %edx, %esp      # esp = to->stack

    # 恢复新任务的状态
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret # 相当于恢复eip，执行完后，任务就正式切换了
