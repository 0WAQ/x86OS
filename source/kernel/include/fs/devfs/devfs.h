/**
 * 
 * devfs头文件
 * 
 */
#ifndef DEVFS_H
#define DEVFS_H

#include "devfs_t.h"
#include "fs/fs_t.h"

// TODO: 实现各种函数

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
int devfs_seek(file_t* file, u32_t offset, int dir);

/**
 * @brief devfs的状态函数
 */
int devfs_stat(file_t* file, struct stat* st);

/**
 * @brief devfs的io控制函数
 */
int devfs_ioctl(file_t* file, int cmd, int arg0, int arg1);

/**
 * @brief devfs的打开目录函数
 */
int devfs_opendir(struct _fs_t* fs, const char* name, DIR* dir);

/**
 * @brief devfs的读取目录函数
 */
int devfs_readdir(struct _fs_t* fs, DIR* dir, struct dirent* dirent);

/**
 * @brief devfs的关闭目录函数
 */
int devfs_closedir(struct _fs_t* fs, DIR* dir);

#endif // DEVFS_H