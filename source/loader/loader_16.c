/**
 *
 * 16位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 */

// 16位代码，必须加上放在开头，以便有些io指令生成为32位
__asm__(".code16gcc");

/**
 * @brief 显示字符串 
 */
static void show_msg(const char* msg) {
    char c;

    while((c = *msg++) != '\0') {
        __asm__ __volatile__(
            "mov $0xe, %%ah""\n\t"
            "mov %[ch], %%al""\n\t"
            "int $0x10"
            :                   // 输出操作数，无
            : [ch]"r"(c)        // 输入操作数，ch建立关联，"r"表示任意寄存器
        );
    }
}

void loader_entry() {
    show_msg("...loading...\n\r");
    for(;;) {}
}