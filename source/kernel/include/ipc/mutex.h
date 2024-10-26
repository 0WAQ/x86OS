/***
 * 
 * 互斥锁相关头文件
 * 
 */
#ifndef MUTEX_H
#define MUTEX_H

#include "mutex_t.h"

/**
 * @brief 初始化互斥锁
 */
void mutex_init(mutex_t* mutex);

/**
 * @brief 上锁
 */
void mutex_lock(mutex_t* mutex);

/**
 * @brief 解锁
 */
void mutex_unlock(mutex_t* mutex);

#endif // MUTEX_H