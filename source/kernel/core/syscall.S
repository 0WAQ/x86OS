/* XXX 调用门实现
# 系统调用的处理函数
    .global exception_handler_syscall
    .extern do_handler_syscall
exception_handler_syscall:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    pushf   # eflags

    # 调用处理函数 do_handler_syscall(syscall_frame_t*)
    mov %esp, %eax
    push %eax   # 将当前栈顶压入栈中, 被强转为syscall_args_t*
    call do_handler_syscall # 在该函数中, 修改了frame->eax的值
    add $4, %esp

    popf
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa

    # 切换栈时cpu会在内核栈压入 用户栈的ss, esp, 调用门设置的5个参数, 用户态的cs, eip
    # 返回时要告诉cpu距离原栈ss, esp的字节数, 否则切换不回原栈
    retf $(5 * 4)   # cs发生改变, 使用远跳转
*/