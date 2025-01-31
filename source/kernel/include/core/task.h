/**
 * 
 * 任务管理头文件
 * 
 */
#ifndef TASK_H
#define TASK_H

#include "task_t.h"
#include "common/elf.h"

/**
 * @brief 初始化任务
 * @param esp 任务的用户栈
 */
int task_init(task_t* task, const char* name, u32_t flag, u32_t entry, u32_t esp);

/**
 * @brief 将任务加入到运行队列中
 */
void task_start(task_t* task);

/**
 * @brief 反初始化任务
 */
int task_uninit(task_t* task);

/**
 * @brief 初始化任务状态段
 * @param esp 任务的用户栈
 */
static int tss_init(task_t* task, u32_t flag, u32_t entry, u32_t esp);

/**
 * @brief 切换任务
 */
static void task_switch_from_to(task_t* from, task_t* to);

/**
 * @brief 切换任务，不使用硬件，在core/task.S中实现 TODO:
 */
void simple_switch(u32_t** from, u32_t* to);

/*==============================================================*/

/**
 * @brief 初始化任务管理器
 */
void task_manager_init();

/**
 * @brief 初始化第一个任务
 */
void first_task_init();

/**
 * @brief 在任务管理器中获取一号任务
 */
static task_t* get_first_task();

/**
 * @brief 获取空闲任务
 */
static task_t* get_idle_task();

/**
 * @brief 空闲任务的执行代码
 */
static void idle_task_entry();

/**
 * @brief 获取当前任务
 */
task_t* get_curr_task();

/**
 * @brief 将任务设置为就绪态
 */
void set_task_ready(task_t* task);

/**
 * @brief 将任务设置为block
 */
void set_task_block(task_t* task);

/**
 * @brief 将任务设置为sleep
 */
void set_task_sleep(task_t* task, u32_t ticks);

/**
 * @brief 将任务从sleep唤醒
 */
void set_task_wakeup(task_t* task);

/**
 * @brief 获取下一个运行的任务s
 */
static task_t* get_next_task();

/**
 * @brief 进行一次任务调度
 */
void task_dispatch();

/**
 * @brief 每次中断时, 都会调用此函数
 */
void task_time_tick();

/**
 * @brief 分配一个任务块
 */
task_t* alloc_task();

/**
 * @brief 释放任务块
 */
void free_task(task_t* task);

/**
 * @brief 将elf文件加载到task中
 */
u32_t load_elf_file(task_t* task, const char* filename, u32_t page_dir);

/**
 * @brief 加载程序头
 */
int load_phdr(int fd, Elf32_Phdr* phdr, u32_t page_dir);

/**
 * @brief 复制进程参数到栈中
 * @attention argv与env在另一个页表里
 */
int copy_args(char* to, u32_t page_dir, int argc, char** argv);

/**
 * @brief 获取当前任务文件打开表的第fd项
 */
file_t* get_task_file(int fd);

/**
 * @brief 将file分配给当前任务的file_table, 并返回fd
 */
int task_alloc_fd(file_t* file);

/**
 * @brief 将当前任务的文件打开表的第fd项置空
 */
void task_remove_fd(int fd);

/**
 * @brief 复制父进程的文件描述符表
 */
void copy_opened_files(task_t* child);

#endif // TASK_H