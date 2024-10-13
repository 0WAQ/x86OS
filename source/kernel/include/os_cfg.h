/**
 * 
 * 内核配置文件
 * 
 */
#ifndef OS_CFG_H
#define OS_CFG_H

// GDT表个数
#define GDT_TABLE_SIZE      (8192)

#define KERNEL_SELECTOR_CS  (1 * 8)
#define KERNEL_SELECTOR_DS  (2 * 8)

#define KERNEL_STACK_SIZE   (8192)  // 8KB

#endif // OS_CFG_H