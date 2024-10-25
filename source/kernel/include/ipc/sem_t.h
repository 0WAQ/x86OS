/**
 * 
 * 信号量基本类型头文件
 * 
 */
#ifndef SEM_H
#define SEM_H

#include "common/types.h"
#include "tools/list.h"

/**
 * @brief 信号量
 */
typedef struct _sem_t {
    uint32_t count;     // 计数
    list_t wait_list;   // 等待队列
}sem_t;

#endif // SEM_H