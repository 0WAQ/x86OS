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
#include "fs/file_t.h"

#define TASK_NAME_SIZE              (32)
#define TASK_TIME_SLICE_DEFAULT     (10)
#define TASK_OFILE_NR               (128)

// 任务工作模式
#define TASK_FLAGS_SYSTEM       (1 << 0)
#define TASK_FLAGS_USER         (0 << 0)

/**
 * 同Linux中的task_struct
 */
typedef struct _task_t {
    // uint32_t* stack;    // 保存esp, simple_switch使用

    // 任务的状态
    enum {
        TASK_CREATED,
        TASK_READY,
        TASK_RUNNING,
        TASK_SLEEP,
        TASK_WAITTING,
        TASK_ZOMBIE
    }state;

    // 记录堆区的位置
    uint32_t heap_start;
    uint32_t heap_end;

    int pid;
    struct _task_t* parent; // 父进程

    tss_t tss;
    int tss_sel;    // 该任务对应的tss段的选择子

    list_node_t run_node;   // task在就绪队列中的节点
    list_node_t wait_node;  // task在等待队列(信号量)中的节点
    list_node_t all_node;   // task在任务队列中的节点

    char name[TASK_NAME_SIZE];    // 任务名称

    // 每个时间片是100ms
    uint32_t slice_ticks;   // 时间片计数(递减)
    uint32_t time_ticks;    // 时间片
    uint32_t sleep_ticks;   // 

    file_t* file_table[TASK_OFILE_NR];      // 任务打开的文件表

    int status;             // 进程退出时的状态

}task_t;

/**
 * 任务管理器
 */
typedef struct _task_manager_t {
    task_t* curr_task;  // 当前运行进程

    list_t ready_list;  // 就绪队列
    list_t task_list;   // 任务队列
    list_t sleep_list;  // 睡眠队列

    task_t first_task;  // 一号进程
    task_t idle_task;   // 空闲进程

    int code_sel;       // 代码段选择子
    int data_sel;       // 数据段选择子

}task_manager_t;

/**
 * @brief 用于给执行execve的进程传递参数
 */
typedef struct _task_args_t {
    uint32_t ret_addr;
    uint32_t argc;
    char** argv;
}task_args_t;

#endif // TASK_T_H