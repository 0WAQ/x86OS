/**
 * 
 * 任务管理相关类型和参数
 * 
 */
#ifndef TASK_T_H
#define TASK_T_H

#include "common/types.h"
#include "tools/list.h"
#include "cpu/cpu_t.h"

#define TASK_NAME_SIZE      (32)

/**
 * 同Linux中的task_struct
 */
typedef struct _task_t {
    // uint32_t* stack;    // 保存esp, simple_switch使用
    tss_t tss;
    int tss_sel;    // 该任务对应的tss段的选择子

    list_node_t run_node;   // task在就绪队列中的节点
    list_node_t all_node;   // task在任务队列中的节点

    char name[TASK_NAME_SIZE];    // 任务名称

    // 任务的状态
    enum {
        TASK_CREATED,
        TASK_READY,
        TASK_RUNNING,
        TASK_SLEEP,
        TASK_WAITTING
    }state;


}task_t;

/**
 * 任务管理器
 */
typedef struct _task_manager_t {
    task_t* curr_task;  // 当前运行进程

    list_t ready_list;  // 就绪队列
    list_t task_list;   // 任务队列

    task_t first_task;  // 

}task_manager_t;

#endif // TASK_T_H