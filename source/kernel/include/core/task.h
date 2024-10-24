/**
 * 
 * 任务管理头文件
 * 
 */
#ifndef TASK_H
#define TASK_H

#include "task_t.h"

/**
 * @brief 初始化任务
 */
int task_init(task_t* task, const char* name, uint32_t entry, uint32_t esp);

/**
 * @brief 初始化任务状态段
 */
int tss_init(task_t* task, uint32_t entry, uint32_t esp);

/**
 * @brief 切换任务
 */
void task_switch_from_to(task_t* from, task_t* to);

/**
 * @brief 切换任务，不使用硬件，在init/init_start.S中实现
 */
void simple_switch(uint32_t** from, uint32_t* to);

/*==============================================================*/

/**
 * @brief 初始化任务管理器
 */
void task_manager_init();

/**
 * @brief 初始化第一个任务
 */
void first_task_init();

/**
 * @brief 在任务管理器中获取一号任务
 */
task_t* get_first_task();

/**
 * @brief 初始化空闲任务
 */
void idle_task_init();

/**
 * @brief 获取空闲任务
 */
task_t* get_idle_task();

/**
 * @brief 空闲任务的执行代码
 */
void idle_task_entry();

/**
 * @brief 获取当前任务
 */
task_t* get_curr_task();

/**
 * @brief 将任务设置为就绪态
 */
void set_task_ready(task_t* task);

/**
 * @brief 将任务设置为block
 */
void set_task_block(task_t* task);

/**
 * @brief 将任务设置为sleep
 */
void set_task_sleep(task_t* task, uint32_t ticks);

/**
 * @brief 将任务从sleep唤醒
 */
void set_task_wakeup(task_t* task);

/**
 * @brief 会让任务主动放弃cpu
 */
int sys_yield();

/**
 * @brief 获取下一个运行的任务s
 */
task_t* get_next_task();

/**
 * @brief 进行一次任务调度
 */
void task_dispatch();

/**
 * @brief 每次中断时, 都会调用此函数
 */
void task_time_tick();

/**
 * @brief 让任务睡眠
 */
void sys_sleep(uint32_t ms);

#endif // TASK_H