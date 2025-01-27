/***
 * 
 * 互斥锁基本类型
 * 
 */
#ifndef MUTEX_T_H
#define MUTEX_T_H

#include "common/types.h"
#include "tools/list.h"
#include "core/task.h"

typedef struct _mutex_t {
    task_t* owner;      // 锁的拥有者
    u32_t count;     // 锁的使用计数
    list_t wait_list;   // 等待队列
}mutex_t;

#endif // MUTEX_T_H