/**
 * 
 * 文件系统头文件
 * 
 */
#ifndef FS_H
#define FS_H

#include "fs_t.h"

/**
 * @brief
 */
int sys_open(const char* filename, int flags, ...);

/**
 * @brief
 */
int sys_read(int fd, char* buf, int len);

/**
 * @brief
 */
int sys_write(int fd, char* buf, int len);

/**
 * @brief
 */
int sys_lseek(int fd, int offset, int dir);

/**
 * @brief
 */
int sys_close(int fd);

#endif // FS_H