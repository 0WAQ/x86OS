/**
 * 
 * 内核初始化及测试代码
 * 
 */
#ifndef INIT_H
#define INIT_H

#include "common/boot_info.h"

/**
 * @brief 内核入口函数
 */
void kernel_init(boot_info_t* boot_info);

/**
 * @brief
 */
void init_main();

#endif // INIT_H