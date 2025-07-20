/**
 * 互斥锁
 */
#ifndef MUTEX_H
#define MUTEX_H

#include "mutex_t.h"

void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);

#endif // MUTEX_H