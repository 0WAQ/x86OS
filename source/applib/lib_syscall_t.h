/**
 * 
 * 系统调用实现相关基本类型
 * 
 */
#ifndef LIB_SYSCALL_T_H
#define LIB_SYSCALL_T_H

#define SYS_sleep       0

typedef struct _syscall_args_t {
    int id;
    int arg0, arg1, arg2, arg3;
}syscall_args_t;


#endif // LIB_SYSCALL_T_H