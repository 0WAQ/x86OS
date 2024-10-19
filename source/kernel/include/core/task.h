/**
 * 
 * 
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

#endif // TASK_H