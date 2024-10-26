/***
 * 
 * 日志输出
 * 
 */
#include "tools/log.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"

// 互斥锁
static mutex_t mutex;

void log_init() {
    // 初始化锁
    mutex_init(&mutex);

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
    
    va_list args; // 声明参数列表
    char buf[256];
    kernel_memset(buf, '\0', sizeof(buf));

    // 初始化参数列表
    va_start(args, fmt);

    // 解析参数列表，将其填充到buf中
    kernel_vsprintf(buf, fmt, args);
    
    // 关闭参数列表
    va_end(args);

    /////////////////////////////////////////// 上锁
    mutex_lock(&mutex);

    const char* p = buf;
    while(*p != '\0') {
        // 忙等待
        while((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }

    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');

    mutex_unlock(&mutex);
    /////////////////////////////////////////// 解锁
}