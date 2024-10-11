
// gcc汇编器默认代码是32位

    .text
    .extern kernel_init
    .global _start
_start:
    call kernel_init
    jmp .
