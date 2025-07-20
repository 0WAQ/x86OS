/**
 * 信号量
 */
#ifndef SEM_H
#define SEM_H

#include "sem_t.h"


void sem_init(sem_t* sem, u32_t count);
void sem_wait(sem_t* sem);
void sem_notify(sem_t* sem);
u32_t sem_count(sem_t* sem);

#endif // SEM_H