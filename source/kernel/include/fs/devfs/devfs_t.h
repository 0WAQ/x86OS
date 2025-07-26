#ifndef DEVFS_T_H
#define DEVFS_T_H

/**
 * @brief 设备文件系统, 用于挂载 tty 等设备
 */
typedef struct _devfs_type_t {
    const char* name;
    int dev_type;
    int file_type;
} devfs_type_t;

#endif // DEVFS_T_H