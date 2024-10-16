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
 * @brief 异常处理程序的执行函数
 */
void do_default_handler(exception_frame_t* frame, const char* msg);


/**
 * @brief unknown的异常处理程序，在init/start.S中实现
 */
void exception_handler_unknown();

/**
 * @brief unknown的异常处理程序，C实现
 */
void do_handler_unknown(exception_frame_t* frame);

#endif // IRQ_H