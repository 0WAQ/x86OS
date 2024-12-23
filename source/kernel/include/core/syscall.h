/**
 * 
 * 系统调用相关头文件
 * 
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "syscall_t.h"

/**
 * @brief 系统调用触发的处理函数
 */
void exception_handler_syscall();
void do_handler_syscall(syscall_frame_t* frame);

#endif // SYSCALL_H