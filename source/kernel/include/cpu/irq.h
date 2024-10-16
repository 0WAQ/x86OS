/***
 * 
 * 中断处理头文件
 * 
 */
#ifndef IRQ_H
#define IRQ_H

#include "irq_t.h"

/**
 * @brief 初始化中断描述符表
 */
void irq_init();

/**
 * @brief 将对应的异常与异常处理函数绑定
 */
int irq_install(int irq_num, irq_handler_t handler);

/**
 * @brief 异常处理程序的执行函数
 */
void do_default_handler(exception_frame_t* frame, const char* msg);


/**
 * @brief 异常处理程序，在init/start.S中实现，与异常号绑定
 */
void exception_handler_unknown();
void exception_handler_divider();

/**
 * @brief 异常处理程序，C实现
 */
void do_handler_unknown(exception_frame_t* frame);
void do_handler_divider(exception_frame_t* frame);


#endif // IRQ_H