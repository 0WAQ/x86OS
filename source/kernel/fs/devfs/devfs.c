/**
 * 
 * dev文件系统c文件
 * 
 */
#include "fs/devfs/devfs.h"
#include "dev/dev.h"

/**
 * @brief 设置devfs的回调函数
 */
fs_op_t devfs_op = {
    .mount = devfs_mount,
    .umount = devfs_umount,
    .open = devfs_open,
    .read = devfs_read,
    .write = devfs_write,
    .close = devfs_close,
    .seek = devfs_seek,
    .stat = devfs_stat
};

int devfs_mount(fs_t* fs, int major, int minor) {
    fs->type = FS_DEVFS;
}

void devfs_umount(fs_t* fs) {

}

int devfs_open(fs_t* fs, const char* filepath, file_t* file) {
    return 0;
}

int devfs_read(char* buf, int size, file_t* file) {
    return dev_read(file->dev_id, file->pos, buf, size);
}

int devfs_write(char* buf, int size, file_t* file) {
    return dev_write(file->dev_id, file->pos, buf, size);
}

void devfs_close(file_t* file) {
    dev_close(file->dev_id);
}

int devfs_seek(file_t* file, uint32_t offset, int dir) {
    return -1;
}

int devfs_stat(file_t* file, struct stat* st) {
    return -1;
}
