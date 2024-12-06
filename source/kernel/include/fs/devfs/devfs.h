/**
 * 
 * devfs头文件
 * 
 */
#ifndef DEVFS_H
#define DEVFS_H

#include "devfs_t.h"
#include "fs/fs_t.h"

/**
 * @brief devfs的挂载函数
 */
int devfs_mount(fs_t* fs, int major, int minor);

/**
 * @brief devfs的取消挂载函数
 */
void devfs_umount(fs_t* fs);

/**
 * @brief devfs的打开函数
 */
int devfs_open(fs_t* fs, const char* filepath, file_t* file);

/**
 * @brief devfs的读取函数
 */
int devfs_read(char* buf, int size, file_t* file);

/**
 * @brief devfs的写入函数
 */
int devfs_write(char* buf, int size, file_t* file);

/**
 * @brief devfs的关闭函数
 */
void devfs_close(file_t* file);

/**
 * @brief devfs的定位函数
 */
int devfs_seek(file_t* file, uint32_t offset, int dir);

/**
 * @brief devfs的状态函数
 */
int devfs_stat(file_t* file, struct stat* st);

#endif // DEVFS_H