/**
 * 
 * 文件系统头文件
 * 
 */
#ifndef FS_H
#define FS_H

#include "fs_t.h"

/**
 * @brief 初始化文件系统
 */
void fs_init();

/**
 * @brief 打开一个文件
 */
int sys_open(const char* filename, int flags, ...);

/**
 * @brief 从fd对应的文件中读取
 */
int sys_read(int fd, char* buf, int len);

/**
 * @brief 向fd对应的文件写
 */
int sys_write(int fd, char* buf, int len);

/**
 * @brief
 */
int sys_lseek(int fd, int offset, int dir);

/**
 * @brief 关闭fd
 */
int sys_close(int fd);

/**
 * @brief
 */
int sys_isatty(int fd);

/**
 * @brief
 */
struct stat;
int sys_fstat(int fd, struct stat* st);

/**
 * @brief 系统调用dup
 */
int sys_dup(int fd);

#endif // FS_H