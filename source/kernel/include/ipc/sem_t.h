#ifndef SEM_T_H
#define SEM_T_H

#include "common/types.h"
#include "tools/list_t.h"

typedef struct _sem_t {
    u32_t count;        // 计数
    list_t wait_list;   // 等待队列
} sem_t;

#endif // SEM_T_H