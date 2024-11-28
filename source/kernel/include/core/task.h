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
 * @param esp 内核栈的栈顶
 */
int task_init(task_t* task, const char* name, uint32_t flag, uint32_t entry, uint32_t esp);

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
int tss_init(task_t* task, uint32_t flag, uint32_t entry, uint32_t esp);

/**
 * @brief 切换任务
 */
void task_switch_from_to(task_t* from, task_t* to);

/**
 * @brief 切换任务，不使用硬件，在core/task.S中实现
 */
void simple_switch(uint32_t** from, uint32_t* to);

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
task_t* get_first_task();

/**
 * @brief 获取空闲任务
 */
task_t* get_idle_task();

/**
 * @brief 空闲任务的执行代码
 */
void idle_task_entry();

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
void set_task_sleep(task_t* task, uint32_t ticks);

/**
 * @brief 将任务从sleep唤醒
 */
void set_task_wakeup(task_t* task);

/**
 * @brief 会让任务主动放弃cpu
 */
int sys_yield();

/**
 * @brief 获取下一个运行的任务s
 */
task_t* get_next_task();

/**
 * @brief 进行一次任务调度
 */
void task_dispatch();

/**
 * @brief 每次中断时, 都会调用此函数
 */
void task_time_tick();

/**
 * @brief 让任务睡眠
 */
void sys_sleep(uint32_t ms);

/**
 * @brief 获取任务的pid
 */
int sys_getpid();

/**
 * @brief 创建子进程
 */
int sys_fork();

/**
 * @brief 分配一个任务块
 */
static task_t* alloc_task();

/**
 * @brief 释放任务块
 */
static void free_task(task_t* task);

/**
 * @brief execve系统调用
 * @attention 会涉及到不同进程空间中数据的传递
 */
int sys_execve(char* path, char** argv, char** env);

/**
 * @brief 将elf文件加载到task中
 */
static uint32_t load_elf_file(task_t* task, const char* filename, uint32_t page_dir);

/**
 * @brief 加载程序头
 */
static int load_phdr(int fd, Elf32_Phdr* phdr, uint32_t page_dir);

/**
 * @brief 复制进程参数到栈中
 * @attention argv与env在另一个页表里
 */
static int copy_args(char* to, uint32_t page_dir, int argc, char** argv);

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
 * @brief exit系统调用的内部实现
 */
void sys_exit(int status);

/**
 * @brief 实现wait系统调用
 */
int sys_wait(int* status);

/**
 * @brief 复制父进程的文件描述符表
 */
static void copy_opened_files(task_t* child);

#endif // TASK_H