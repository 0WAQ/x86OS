/***
 * 
 * 字符串的处理函数相关头文件
 *  
 */
#ifndef KLIB_H
#define KLIB_H

#include <stdarg.h>
#include "common/types.h"

/**
 * @brief 处理可变参数列表
 */
void kernel_vsprintf(char* buf, const char* fmt, va_list args);
void kernel_sprintf(char* buf, const char* fmt, ...);

/**
 * @brief 将num以base进制转换为字符串放入dest
 */
void kernel_itoa(char* dest, int num, uint32_t base);

/**
 * @brief 复制字符串
 */
void kernel_strcpy(char* dest, const char* src);
void kernel_strncpy(char* dest, const char* src, int size);

/**
 * @brief 比较字符串
 */
int kernel_strncmp(const char* s1, const char* s2, int size);

/**
 * @brief 获取字符串长度
 */
int kernel_strlen(const char* str);

/**
 * @brief 按字节拷贝
 */
void kernel_memcpy(void* dest, void* src, int size);

/**
 * @brief 初始化
 */
void kernel_memset(void* dest, uint8_t v, int size);

/**
 * @brief 比较
 */
int kernel_memcmp(void* d1, void* d2, int size);

#endif // KLIB_H