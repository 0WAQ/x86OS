#ifndef LIB_SYSCALL_T_H
#define LIB_SYSCALL_T_H

/**
 * @brief 系统调用的参数
 */
typedef struct _syscall_args_t {
    int arg0;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
}syscall_args_t;

#endif // LIB_SYSCALL_T_H