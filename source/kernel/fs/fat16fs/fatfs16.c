/**
 * 
 * FAT16文件系统源文件
 * 
 */
#include "fs/fat16fs/fat16fs.h"
#include "fs/fs.h"
#include "dev/dev.h"
#include "core/memory.h"
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

int fat16fs_mount(struct _fs_t* fs, int major, int minor) {
    
    // 打开设备
    int dev_id = dev_open(major, minor, NULL);
    if(dev_id < 0) {
        log_print("open disk failed, major: %x, minor: %x", major, minor);
        return -1;
    }

    // 分配dbr缓冲区
    dbr_t* dbr = (dbr_t*)memory_alloc_page();
    if(dbr == NULL) {
        log_print("mount failed, can't alloc buf");
        goto fat16fs_mount_failed;
    }

    // 读取DBR到dbr缓冲区中
    int cnt = dev_read(dev_id, 0, (char*)dbr, 1);
    if(cnt < 1) {
        log_print("read dbr failed.");
        goto fat16fs_mount_failed;
    }

    // 解析DBR参数
    fat16_t* fat = &fs->fat16_data;
    fat->bytes_per_sectors = dbr->BPB_BytsPerSec;
    fat->sectors_per_clusters = dbr->BPB_SecPerClus;

    fat->fat_tbl_start = dbr->BPB_RsvdSecCnt;
    fat->fat_tbl_sectors = dbr->BPB_FATSz16;
    fat->fat_tbl_cnt = dbr->BPB_NumFATs;

    fat->root_start = fat->fat_tbl_start + fat->fat_tbl_sectors * fat->fat_tbl_cnt;
    fat->root_entry_cnt = dbr->BPB_RootEntCnt;
    
    fat->data_start = fat->root_start + fat->root_entry_cnt * 32 / SECTOR_SIZE;
    
    fat->fat_buffer = (uint8_t*)dbr;
    fat->clusters_byte_size = fat->sectors_per_clusters * dbr->BPB_BytsPerSec;
    fat->fs = fs;

    // 正确性检验
    if(fat->fat_tbl_cnt != 2) {
        log_print("fat table error, major: %x, minor: %x", major, minor);
        goto fat16fs_mount_failed;
    }

    if(kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_print("not a fat16 file system, major: %x, minor: %x", major, minor);
        goto fat16fs_mount_failed;
    }

    // 初始化
    fs->type = FS_FAT16;
    fs->data = &fs->fat16_data;
    fs->dev_id = dev_id;
    
    return 0;

fat16fs_mount_failed:
    if(dbr != NULL) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
    return -1;
}

void fat16fs_umount(struct _fs_t* fs) {
    fat16_t* fat = (fat16_t*)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}

int fat16fs_open(struct _fs_t* fs, const char* filepath, file_t* file) {
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
