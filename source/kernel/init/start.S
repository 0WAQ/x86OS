
// gcc汇编器默认代码是32位

    .text
    .extern kernel_init

    // void _start(boot_info_t* boot_info)
    .global _start
_start:
    // 将 boot_info 压栈
    // push %ebp
    // mov %esp, %ebp
    // mov 0x8(%ebp), %eax
    mov 0x4(%esp), %eax

    // kernel_init(boot_info)
    push %eax
    call kernel_init
    jmp .
