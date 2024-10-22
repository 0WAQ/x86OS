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
 * @brief 将任务设置为就绪态
 */
void set_task_ready(task_t* task);

/**
 * @brief 将任务设置为
 */
void set_task_block(task_t* task);

#endif // TASK_H