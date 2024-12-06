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
 * @brief 定位fd的读取写入位置
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

/**
 * @brief 初始化空闲与挂载链表
 */
static void mount_list_init();

/**
 * @brief 根据fs类型, 获取对应的操作函数
 */
static fs_op_t* get_fs_op(fs_type_t type, int major);

/**
 * @brief 将一个type类型的fs挂载到mount_point
 */
static fs_t* mount(fs_type_t type, char* mount_point, int dev_major, int dev_minor);

#endif // FS_H