#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
#define NULL        ((void*)0)
#endif  // NULL

typedef unsigned char u8_t;
typedef signed char i8_t;
typedef unsigned short u16_t;
typedef signed short i16_t;
typedef unsigned int u32_t;
typedef signed int i32_t;
typedef const char* string;

typedef u32_t bool;
#define true ((u32_t)1)
#define false ((u32_t)0)

#endif // TYPES_H