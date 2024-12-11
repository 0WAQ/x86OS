/**
 * 
 * devfs相关基本类型
 * 
 */
#ifndef DEVFS_T_H
#define DEVFS_T_H

/**
 * @brief
 */
typedef struct _devfs_type_t {
    const char* name;
    int dev_type;
    int file_type;
}devfs_type_t;

#endif // DEVFS_T_H