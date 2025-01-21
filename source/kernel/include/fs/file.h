/**
 * 
 * 文件相关头文件
 * 
 */
#ifndef FILE_H
#define FILE_H

#include "fs/file_t.h"

/**
 * @brief 初始化文件表
 */
void file_table_init();

/**
 * @brief 分配一个文件
 */
file_t* file_alloc();

/**
 * @brief 介绍file的引用计数(计数为0时释放file)
 */
void file_dec_ref(file_t* file);

/**
 * @brief 增加file的引用计数
 */
void file_inc_ref(file_t* file);


#endif // FILE_H
