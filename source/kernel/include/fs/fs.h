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

/**
 * @brief 寻找path中下一个子路径
 */
const char* path_next_child(const char* path);

/**
 * @brief 将path转换为int类型
 */
int string_to_int(const char* path, int* num);

/**
 * @brief 判断path是否以str开头
 */
int path_begin_with(const char* path, const char* str);

/**
 * @brief 判断fd是否有效
 */
int is_fd_bad(int fd);

/**
 * @brief 为fs上锁
 */
void fs_lock(fs_t* fs);

/**
 * @brief 为fs释放锁
 */
void fs_unlock(fs_t* fs);

#endif // FS_H