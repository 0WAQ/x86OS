/**
 * 
 * 文件相关基本类型
 * 
 */
#ifndef FILE_T_H
#define FILE_T_H

#include "common/types.h"

#define FILENAME_SIZE           (32)
#define FILE_TABLE_NR           (2048)

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
    uint32_t size;                  // 文件大小
    int ref;                        // 打开计数
    int dev_id;                     // 该文件对应的设备id
    int pos;                        // 当前读取的位置
    int mode;                       // 文件相关属性

    struct _fs_t* fs;               // 

}file_t;

#endif // FILE_T_H
