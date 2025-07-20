/**
 * 硬件定时器配置头文件
 */
#ifndef TIMER_H
#define TIMER_H

#include "dev/timer_t.h"
#include "cpu/irq_t.h"

/**
 * @brief 初始化定时器
 */
void timer_init();

/**
 * @brief 初始化硬件定时器
 */
static void pit_init();

/**
 * @brief 定时器中断处理函数
 */
void exception_handler_timer();
void do_handler_timer(exception_frame_t* frame);

static int dummy_timer_h = PIT_OSC_FREQ;

#endif // TIMER_H