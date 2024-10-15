/***
 * 
 * 中断处理头文件
 * 
 */
#ifndef IRQ_H
#define IRQ_H

/**
 * @brief 初始化中断描述符表
 */
void irq_init();

/**
 * @brief 默认中断处理程序，在init/start.S中实现
 */
void default_exception_handler();

/**
 * @brief 默认中断处理程序，C实现
 */
void default_exception_handler_aux();

/**
 * @brief 默认中断处理程序的执行函数
 */
void do_default_handler(const char* msg);

#endif // IRQ_H