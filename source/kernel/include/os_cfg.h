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

#define KERNEL_STACK_SIZE   (8192)  // 8KB

#endif // OS_CFG_H