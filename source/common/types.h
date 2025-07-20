/**
 * 系统内部的基本数据类型
 */
#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
#define NULL        ((void*)0)
#endif  // NULL

typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned int u32_t;
typedef signed int s32_t;
typedef const char* string;

typedef u32_t bool;
#define true ((u32_t)1)
#define false ((u32_t)0)

#endif // TYPES_H