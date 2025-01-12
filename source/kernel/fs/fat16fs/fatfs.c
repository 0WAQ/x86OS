/**
 * 
 * FAT16文件系统源文件
 * 
 */
#include "fs/fat16fs/fat16fs.h"
#include "fs/fs.h"
#include "dev/dev.h"
#include "tools/klib.h"
#include "tools/log.h"

/**
 * @brief 设置fat16文件系统的回调函数
 */
fs_op_t fat16fs_op = {
    .mount = fat16fs_mount,
    .umount = fat16fs_umount,
    .open = fat16fs_open,
    .read = fat16fs_read,
    .write = fat16fs_write,
    .close = fat16fs_close,
    .seek = fat16fs_seek,
    .stat = fat16fs_stat
};

int fat16fs_mount(fs_t* fs, int major, int minor) {
    return -1;
}

void fat16fs_umount(fs_t* fs) {

}

int fat16fs_open(fs_t* fs, const char* filepath, file_t* file) {
    return -1;
}

int fat16fs_read(char* buf, int size, file_t* file) {
    return 0;
}

int fat16fs_write(char* buf, int size, file_t* file) {
    return 0;
}

void fat16fs_close(file_t* file) {

}

int fat16fs_seek(file_t* file, uint32_t offset, int dir) {
    return -1;
}

int fat16fs_stat(file_t* file, struct stat* st) {
    return -1;
}
