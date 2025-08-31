#ifndef KLIB_H
#define KLIB_H

#include <stdarg.h>
#include "common/types.h"

/**
 * @brief 将size向下(小)对齐到bound的整数倍
 */
static inline 
u32_t kdown2(u32_t len, u32_t bound) {
    return len & ~(bound - 1);
}

/**
 * @brief 将size向上(大)对齐到bound的整数倍
 */
static inline 
u32_t kup2(u32_t len, u32_t bound) {
    return (len + bound - 1) & ~(bound - 1);
}

/**
 * @brief 处理可变参数列表
 */
void kvsprintf(char* buf, const char* fmt, va_list args);
void ksprintf(char* buf, const char* fmt, ...);

/**
 * @brief 将num以base进制转换为字符串放入dest
 */
void kitoa(char* dest, int num, u32_t base);

void kstrcpy(char* dest, const char* src);
void kstrncpy(char* dest, const char* src, int len);

/**
 * @brief 比较字符串
 * @return s1 > s2 则返回大于0的值;
 *         s1 < s2 则返回小于0的值;
 *         s1 = s2 则返回0
 */
int kstrcmp(const char* s1, const char* s2);
int kstrncmp(const char* s1, const char* s2, int len);

int kstrlen(const char* str);
void kmemcpy(void* dest, const void* src, int len);
void kmemset(void* dest, u8_t v, int len);
int kmemcmp(void* d1, void* d2, int len);

/**
 * @brief ASSERT相关
 */
void panic(const char* file, int line, const char* func, const char* msg);

#ifndef RELEASE
    #define ASSERT(expr)    if(!(expr)) panic(__FILE__, __LINE__, __func__, #expr);
#else   // DEBUG
    #define ASSERT(expr)    ((void)0)
#endif // RELEASE

u32_t strings_count(char** addr);
const char* get_filename_from_path(const char* path);

#endif // KLIB_H