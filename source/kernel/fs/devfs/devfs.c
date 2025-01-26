/**
 * 
 * dev文件系统c文件
 * 
 */
#include "fs/devfs/devfs.h"
#include "fs/fs.h"
#include "dev/dev.h"
#include "tools/klib.h"
#include "tools/log.h"
#include <sys/fcntl.h>

/**
 * @brief 设置devfs的回调函数
 */
fs_op_t devfs_op = {
    .mount = devfs_mount,
    .umount = devfs_umount,
    .unlink = NULL,
    .open = devfs_open,
    .read = devfs_read,
    .write = devfs_write,
    .close = devfs_close,
    .seek = devfs_seek,
    .stat = devfs_stat,
    .ioctl = devfs_ioctl,
    .opendir = devfs_opendir,
    .readdir = devfs_readdir,
    .closedir = devfs_closedir
};

/**
 * @brief 设备类型表
 */
static devfs_type_t dev_type_table[] = {
    {
        .name = "tty",
        .dev_type = DEV_TTY,
        .file_type = FILE_TTY,
    },
    {
        .name = "disk",
        .dev_type = DEV_DISK,
        .file_type = FILE_UNKNOWN,
    },
    {
        .name = "timer",
        .dev_type = DEV_TIMER,
        .file_type = FILE_UNKNOWN,
    }
};

int devfs_mount(fs_t* fs, int major, int minor) {
    // TODO:
    return 0;
}

void devfs_umount(fs_t* fs) {
    // TODO:
    return;
}

int devfs_open(fs_t* fs, const char* filepath, file_t* file) {

    // 从devfs_type_table中查找相同的文件路径
    for(int i = 0; i < sizeof(dev_type_table)/sizeof(devfs_type_t); i++) {
        devfs_type_t* type = dev_type_table + i;
        int type_name_len = kernel_strlen(type->name);
        
        // 若相同
        if(kernel_strncmp(filepath, type->name, type_name_len) == 0) {
            int minor;
            if((kernel_strlen(filepath) > type_name_len) && 
               (string_to_int(filepath + type_name_len, &minor) < 0)) {
                
                log_print("Get device num failed. %s", filepath);
                break;
            }

            int dev_id = dev_open(type->dev_type, minor, NULL);
            if(dev_id < 0) {
                log_print("Open device failed: %s.", filepath);
                break;
            }

            file->dev_id = dev_id;
            file->fs = fs;
            file->pos = 0;
            file->size = 0;
            file->type = type->file_type;
            return 0;
        }
    }

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
    // TODO:
    return -1;
}

int devfs_stat(file_t* file, struct stat* st) {
    if(file == NULL || st == NULL) {
        log_print("invalid file or stat structure.");
        return -1;
    }

    kernel_memset(st, 0, sizeof(struct stat));

    st->st_mode = 0;
    switch (file->type) {
        case FILE_TTY:
            st->st_mode |= S_IFCHR; // 字符设备
            break;
        default:
            st->st_mode |= S_IFREG; // 普通文件
            break;
    }

    st->st_size = file->size;   // 文件大小
    st->st_rdev = file->dev_id; // 设备号
    st->st_ino = file->dev_id;  // inode号, 直接使用设备号 TODO:
    st->st_nlink = 1;           // 硬链接数, 设备文件通常为1
    st->st_mode |= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;   // 文件权限
    st->st_atime = st->st_mtime = st->st_ctime = 0; // 时间戳 TODO:

    return 0;
}

int devfs_ioctl(file_t* file, int cmd, int arg0, int arg1) {
    return dev_control(file->dev_id, cmd, arg0, arg1);
}

int devfs_opendir(struct _fs_t* fs, const char* name, DIR* dir) {
    if(dir == NULL) {
        log_print("open directory failed.");
        return -1;
    }
    dir->index = 0;
    return 0;
}

int devfs_readdir(struct _fs_t* fs, DIR* dir, struct dirent* dirent) {
    if(dir->index++ < 10) {
        dirent->type = FILE_NORMAL;
        dirent->size = 1000;
        kernel_memcpy(dirent->name, "devfs", sizeof(dirent->name));
        return 0;
    }
    return -1;
}

int devfs_closedir(struct _fs_t* fs, DIR* dir) {
    // TODO:
    return 0;
}
