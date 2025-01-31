/**
 * 
 * 系统调用的外部相关基本类型
 * 
 */
#ifndef LIB_SYSCALL_T_H
#define LIB_SYSCALL_T_H

#include "common/types.h"

/**
 * @brief 系统调用的参数
 */
typedef struct _syscall_args_t {
    int arg0, arg1, arg2, arg3, arg4;
}syscall_args_t;

#endif // LIB_SYSCALL_T_H