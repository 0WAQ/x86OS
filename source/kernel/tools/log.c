/***
 * 
 * 日志输出
 * 
 */
#include "tools/log.h"
#include "common/cpu_instr.h"

void log_init() {
    
    // 将串行接口相关的中断关闭
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0F);
}

void log_print(const char* fmt, ...) {
    const char* p = fmt;
    while(*p != '\0') {
        // 忙等待
        while((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }

    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
}