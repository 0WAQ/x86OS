/**
 * 
 * 硬件定时器配置头文件
 * 
 */
#ifndef TIME_H
#define TIME_H

#include "timer_t.h"
#include "cpu/cpu_t.h"
#include "cpu/irq.h"

/**
 * @brief 初始化定时器
 */
void timer_init();

/**
 * @brief 初始化硬件定时器
 */
void pit_init();

/**
 * @brief 定时器中断处理函数
 */
void exception_handler_timer();
void do_handler_timer(exception_frame_t* frame);


#endif // TIME_H