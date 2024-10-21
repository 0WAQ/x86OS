/**
 * 
 * 任务管理相关类型和参数
 * 
 */
#ifndef TASK_T_H
#define TASK_T_H

#include "common/types.h"
#include "cpu/cpu_t.h"

/**
 * 同Linux中的task_struct
 */
typedef struct _task_t {
    // uint32_t* stack;    // 保存esp, simple_switch使用`
    tss_t tss;
    int tss_sel;    // 该任务对应的tss段的选择子
}task_t;

#endif // TASK_T_H