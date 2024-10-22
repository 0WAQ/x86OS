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
int task_init(task_t* task, uint32_t entry, uint32_t esp);

/**
 * @brief 初始化任务状态段
 */
int tss_init(task_t* task, uint32_t entry, uint32_t esp);

/**
 * @brief 切换任务
 */
void task_switch_from_to(task_t* from, task_t* to);

/**
 * @brief 切换任务，不使用硬件
 */
void simple_switch(uint32_t** from, uint32_t* to);

/*==============================================================*/

/**
 * @brief
 */
void task_manager_init();

/**
 * @brief
 */
void first_task_init();


/**
 * @brief
 */
task_t* get_first_task();

#endif // TASK_H