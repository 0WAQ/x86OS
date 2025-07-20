/**
 * devfs
 */
#ifndef DEVFS_H
#define DEVFS_H

#include "fs/fs_t.h"

// TODO: 实现各种函数

static int devfs_mount(fs_t* fs, int major, int minor);
static void devfs_umount(fs_t* fs);

static int devfs_open(fs_t* fs, const char* filepath, file_t* file);
static int devfs_read(char* buf, int size, file_t* file);
static int devfs_write(char* buf, int size, file_t* file);
static void devfs_close(file_t* file);

static int devfs_seek(file_t* file, u32_t offset, int dir);
static int devfs_stat(file_t* file, struct stat* st);
static int devfs_ioctl(file_t* file, int cmd, int arg0, int arg1);

static int devfs_opendir(struct _fs_t* fs, const char* name, DIR* dir);
static int devfs_readdir(struct _fs_t* fs, DIR* dir, struct dirent* dirent);
static int devfs_closedir(struct _fs_t* fs, DIR* dir);

#endif // DEVFS_H