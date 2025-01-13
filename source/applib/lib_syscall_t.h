/**
 * 
 * 系统调用的外部相关基本类型
 * 
 */
#ifndef LIB_SYSCALL_T_H
#define LIB_SYSCALL_T_H

#include "common/types.h"

/**
 * @brief
 */
typedef struct _syscall_args_t {
    int id;
    int arg0, arg1, arg2, arg3;
}syscall_args_t;

/**
 * @brief
 */
struct dirent {
    int index;
    int type;
    char name[255];
    int size;
};

/**
 * @brief
 */
typedef struct _DIR {
    int index;
    struct dirent dirent;
}DIR;

#endif // LIB_SYSCALL_T_H