/**
 * 
 * 文件系统基本类型
 * 
 */
#ifndef FS_T_H
#define FS_T_H

#include "common/types.h"
#include "file.h"
#include "sys/stat.h"
#include "ipc/mutex.h"
#include "fat16fs/fat16fs_t.h"
#include "applib/lib_syscall_t.h"
#include "tools/list.h"

#define FS_MOUNTP_SIZE      (512)
#define FS_TABLE_SIZE       (10)

struct _fs_t;
typedef struct _fs_t fs_t;

/**
 * @brief 文件系统的回调函数
 */
typedef struct _fs_op_t {
    int  (*mount)(fs_t* fs, int major, int minor);
    void (*umount)(fs_t* fs);
    int  (*unlink)(fs_t* fs, const char* filename);
    int  (*open)(fs_t* fs, const char* filepath, file_t* file);
    int  (*read)(char* buf, int size, file_t* file);
    int  (*write)(char* buf, int size, file_t* file);
    void (*close)(file_t* file);
    int  (*seek)(file_t* file, uint32_t offset, int dir);
    int  (*stat)(file_t* file, struct stat* st);
    int  (*ioctl)(file_t* file, int cmd, int arg0, int arg1);
    int  (*opendir)(struct _fs_t* fs, const char* name, DIR* dir);
    int  (*readdir)(struct _fs_t* fs, DIR* dir, struct dirent* dirent);
    int  (*closedir)(struct _fs_t* fs, DIR* dir);
}fs_op_t;

/**
 * @brief 文件系统类型
 */
typedef enum _fs_type_t {
    FS_DEVFS,
    FS_FAT16,
}fs_type_t;

/**
 * @brief 文件系统结构
 */
typedef struct _fs_t {
    char mount_point[FS_MOUNTP_SIZE];   // 挂载点
    fs_type_t type;                     // fs类型
    fs_op_t* op;                        // fs对应的操作函数
    void* data;
    int dev_id;
    list_node_t node;                   // fs在挂载链表中的节点
    mutex_t* mtx;

    union {
        fat16_t fat16_data;
    };

}fs_t;

#endif // FS_T_H