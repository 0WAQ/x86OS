/**
 * 
 * FAT16文件系统头文件
 * 
 */
#ifndef FAT16FS_H
#define FAT16FS_H

#include "fat16fs_t.h"
#include "fs/fs_t.h"
#include "ipc/mutex.h"

/**
 * @brief fat16fs的挂载函数
 */
int fat16fs_mount(struct _fs_t* fs, int major, int minor);

/**
 * @brief fat16fs的取消挂载函数
 */
void fat16fs_umount(struct _fs_t* fs);

/**
 * @brief fa16fs的解引用函数
 */
int fat16fs_unlink(struct _fs_t* fs, const char* filename);

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
int fat16fs_seek(file_t* file, u32_t offset, int dir);

/**
 * @brief fat16fs的状态函数
 */
int fat16fs_stat(file_t* file, struct stat* st);

/**
 * @brief fat16fs的打开目录函数
 */
int fat16fs_opendir(struct _fs_t* fs, const char* name, DIR* dir);

/**
 * @brief fat16fs的读取目录函数
 */
int fat16fs_readdir(struct _fs_t* fs, DIR* dir, struct dirent* dirent);

/**
 * @brief fat16fs的关闭目录函数
 */
int fat16fs_closedir(struct _fs_t* fs, DIR* dir);

/**
 * @brief 从fat16fs中读取第index个目录项
 */
static diritem_t* read_dir_entry(fat16_t* fat, int index);

/**
 * @brief 向fat16fs的第index个目录项写入dir
 */
static int write_dir_entry(fat16_t* fat, diritem_t* dir, int index);

/**
 * @brief 解析目录项类型
 */
static file_type_t diritem_parse_type(diritem_t* item);

/**
 * @brief 解析目录项名称
 */
static void diritem_parse_name(diritem_t* item, char* dest);

/**
 * @brief 从sector读取数据, 带缓冲
 */
static int bread_sector(fat16_t* fat, int sector);

/**
 * @brief 向sector写入数据, 无需缓冲
 */
static int bwrite_sector(fat16_t* fat, int sector);

/**
 * @brief 
 */
static void read_from_diritem(fat16_t* fat, file_t* file, diritem_t* item, int index);


/**
 * @brief 初始化diritem
 */
static int diritem_init(diritem_t* item, const char* name, u8_t attr);

/**
 * @brief 比较name与目录项的Name
 */
static int diritem_name_match(diritem_t* item, const char* name);

/**
 * @brief 将diritem中的DIR_Name转换成正常
 */
static void to_sfn(char* dest, const char* src);

/**
 * @brief 调整文件的pos
 */
static int move_file_pos(file_t* file, fat16_t* fat, u32_t move_bytes, int expand);

/**
 * @brief 判断簇是否有效
 */
static inline int cluster_is_valid(cluster_t cluster) {
    return ((cluster < FAT_CLUSTER_INVALID) && (cluster >= 0x2));
}

/**
 * @brief 获取下一簇
 */
static cluster_t cluster_get_next(fat16_t* fat, cluster_t curr);

/**
 * @brief 删除以begin开始的簇链
 */
static void cluster_free_chain(fat16_t* fat, cluster_t begin);

/**
 * @brief 扩充文件
 */
static int expand_file(file_t* file, int inc);

/**
 * @brief 分配簇
 */
static cluster_t cluster_alloc(fat16_t* fat, int cnt);

#endif // FAT16FS_H