/**
 * 
 * 内核配置文件
 * 
 */
#ifndef OS_CFG_H
#define OS_CFG_H

// GDT表个数
#define GDT_TABLE_SIZE      (8192)

// IDT表个数，0～31内部保留
#define IDT_TABLE_SIZE      (256)

#define KERNEL_SELECTOR_CS  (1 * 8)
#define KERNEL_SELECTOR_DS  (2 * 8)
#define SELECTOR_SYSCALL    (3 * 8)

#define KERNEL_STACK_SIZE   (8192)  // 8KB
#define IDLE_STACK_SIZE     (1024)

// 每毫秒的时钟数
#define OS_TICK_MS      10


#define OS_VERSION      "1.0.0"

#endif // OS_CFG_H