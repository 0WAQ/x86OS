/**
 * 
 * 文件相关基本类型
 * 
 */
#ifndef FILE_T_H
#define FILE_T_H

#include "common/types.h"

#define FILENAME_SIZE           (32)
#define SYS_OFILE_NR           (2048)   // 系统中最多打开的文件数

/**
 * @brief 文件类型
 */
typedef enum _file_type_t {
    FILE_UNKNOWN = 0,
    FILE_TTY,
    FILE_DIR,
    FILE_NORMAL
}file_type_t;

struct _fs_t;

/**
 * @brief 文件结构
 */
typedef struct _file_t {
    char filename[FILENAME_SIZE];   // 文件名
    file_type_t type;               // 文件类型
    u32_t size;                  // 文件大小
    int ref;                        // 打开计数
    int dev_id;                     // 该文件对应的设备id
    int pos;                        // 当前读取的位置
    int mode;                       // 文件相关属性
    int sblk;                       // 文件起始的块号
    int cblk;                       // 当前读取的块号
    // TODO: time
    struct _fs_t* fs;               // 
    int p_index;
}file_t;

#endif // FILE_T_H
