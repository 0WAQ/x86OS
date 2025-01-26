/**
 * 
 * 文件系统头文件
 * 
 */
#ifndef FS_H
#define FS_H

#include "fs_t.h"
#include "applib/lib_syscall_t.h"

/**
 * @brief 初始化文件系统
 */
void fs_init();

/**
 * @brief 系统调用: 删除文件(减少引用计数)
 */
int sys_unlink(const char* filename);

/**
 * @brief 系统调用: 打开一个文件
 */
int sys_open(const char* filename, int flags, ...);

/**
 * @brief 系统调用: 从fd对应的文件中读取
 */
int sys_read(int fd, char* buf, int len);

/**
 * @brief 系统调用: 向fd对应的文件写
 */
int sys_write(int fd, char* buf, int len);

/**
 * @brief 系统调用: 定位fd的读取写入位置
 */
int sys_lseek(int fd, int offset, int dir);

/**
 * @brief 系统调用: 关闭fd
 */
int sys_close(int fd);

/**
 * @brief 系统调用: 判断fd是否是一个tty设备
 */
int sys_isatty(int fd);

/**
 * @brief 系统调用: 返回文件状态信息
 */
struct stat;
int sys_fstat(int fd, struct stat* st);

/**
 * @brief 系统调用: 对输入输出设备进行控制 TODO: 将其改为可变参传递(前提: 将系统调用改为可变参传递)
 */
int sys_ioctl(int fd, int cmd, int arg0, int arg1);

/**
 * @brief 系统调用: 拷贝(增加引用计数)fd对应的file
 */
int sys_dup(int fd);

/**
 * @brief 系统调用: 打开目录
 */
int sys_opendir(const char* path, DIR* dir);

/**
 * @brief 系统调用: 读取目录
 */
int sys_readdir(DIR* dir, struct dirent* dirent);

/**
 * @brief 系统调用: 关闭目录
 */
int sys_closedir(DIR* dir);


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
static int is_fd_bad(int fd);

/**
 * @brief 为fs上锁
 */
static void fs_lock(fs_t* fs);

/**
 * @brief 为fs释放锁
 */
static void fs_unlock(fs_t* fs);

#endif // FS_H