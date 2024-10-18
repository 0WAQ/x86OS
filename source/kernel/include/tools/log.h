/**
 * 
 * 日志输出相关头文件
 * 
 */
#ifndef LOG_H
#define LOG_H

#include <stdarg.h>     // 该头文件不依赖任何库，只使用了编译器内置方法
#include "log_t.h"

/**
 * @brief 初始化日志
 */
void log_init();

/**
 * @brief 输出，作用同printf
 */
void log_print(const char* fmt, ...);

#endif // LOG_H