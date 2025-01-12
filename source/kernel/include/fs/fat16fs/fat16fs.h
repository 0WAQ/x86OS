/**
 * 
 * FAT16文件系统头文件
 * 
 */
#ifndef FAT16FS_H
#define FAT16FS_H

#include "fat16fs_t.h"
#include "fs/fs_t.h"

/**
 * @brief fat16fs的挂载函数
 */
int fat16fs_mount(struct _fs_t* fs, int major, int minor);

/**
 * @brief fat16fs的取消挂载函数
 */
void fat16fs_umount(struct _fs_t* fs);

/**
 * @brief fat16fs的打开函数
 */
int fat16fs_open(struct _fs_t* fs, const char* filepath, file_t* file);

/**
 * @brief fat16fs的读取函数
 */
int fat16fs_read(char* buf, int size, file_t* file);

/**
 * @brief fat16fs的写入函数
 */
int fat16fs_write(char* buf, int size, file_t* file);

/**
 * @brief fat16fs的关闭函数
 */
void fat16fs_close(file_t* file);

/**
 * @brief fat16fs的定位函数
 */
int fat16fs_seek(file_t* file, uint32_t offset, int dir);

/**
 * @brief fat16fs的状态函数
 */
int fat16fs_stat(file_t* file, struct stat* st);

#endif // FAT16FS_H