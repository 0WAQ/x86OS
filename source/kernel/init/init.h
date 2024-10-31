/**
 * 
 * 内核初始化及测试代码
 * 
 */
#ifndef INIT_H
#define INIT_H

#include "common/boot_info.h"

/**
 * @brief 内核初始化函数
 */
void kernel_init(boot_info_t* boot_info);

/**
 * @brief 内核main函数
 */
void kernel_main();

/**
 * @brief 运行1号进程
 */
void move_to_first_task();

#endif // INIT_H