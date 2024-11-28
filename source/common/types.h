/**
 * 
 * 系统内部的基本数据类型
 * 
 */
#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
#define NULL        ((void*)0)
#endif  // NULL

/**
 * 使用宏处理的原因:
 * 为避免和newlib库中的定义冲突
 */
#ifndef _UINT8_T_DECLARED
#define _UINT8_T_DECLARED
typedef unsigned char uint8_t;
#endif // _UINT8_T_DECLARED

#ifndef _UINT16_T_DECLARED
#define _UINT16_T_DECLARED
typedef unsigned short uint16_t;
#endif // _UINT16_T_DECLARED

#ifndef _UINT32_T_DECLARED
#define _UINT32_T_DECLARED
typedef unsigned int uint32_t;
#endif // _UINT32_T_DECLARED

#endif // TYPES_H