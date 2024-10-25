/**
 * 
 * 信号量头文件
 * 
 */
#ifndef SEM_T_H
#define SEM_T_H

#include "sem_t.h"

/**
 * @brief 初始化信号量
 */
void sem_init(sem_t* sem, uint32_t count);

/**
 * @brief 将任务阻塞在该信号量上
 */
void sem_wait(sem_t* sem);

/**
 * @brief 将任务从该信号量上唤醒
 */
void sem_notify(sem_t* sem);

/**
 * @brief 获取信号量的引用计数
 */
uint32_t sem_count(sem_t* sem);

#endif // SEM_T_H