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
#include <sys/fcntl.h>

/**
 * @brief 设置fat16文件系统的回调函数
 */
fs_op_t fat16fs_op = {
    .mount = fat16fs_mount,
    .umount = fat16fs_umount,
    .unlink = fat16fs_unlink,
    .open = fat16fs_open,
    .read = fat16fs_read,
    .write = fat16fs_write,
    .close = fat16fs_close,
    .seek = fat16fs_seek,
    .stat = fat16fs_stat,
    .ioctl = NULL,
    .opendir = fat16fs_opendir,
    .readdir = fat16fs_readdir,
    .closedir = fat16fs_closedir
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
    fat->bytes_per_sector = dbr->BPB_BytsPerSec;
    fat->sectors_per_cluster = dbr->BPB_SecPerClus;

    fat->fat_tbl_start = dbr->BPB_RsvdSecCnt;
    fat->fat_tbl_sectors = dbr->BPB_FATSz16;
    fat->fat_tbl_cnt = dbr->BPB_NumFATs;

    fat->root_start = fat->fat_tbl_start + fat->fat_tbl_sectors * fat->fat_tbl_cnt;
    fat->root_entry_cnt = dbr->BPB_RootEntCnt;
    
    fat->data_start = fat->root_start + fat->root_entry_cnt * 32 / SECTOR_SIZE;
    
    fat->fat_buffer = (uint8_t*)dbr;
    fat->cluster_bytes_size = fat->sectors_per_cluster * dbr->BPB_BytsPerSec;
    fat->last_sector = -1;
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

int fat16fs_unlink(struct _fs_t* fs, const char* filename) {
    fat16_t* fat = (fat16_t*)fs->data;

    for(int i = 0; i < fat->root_entry_cnt; i++) {
        diritem_t* p = read_dir_entry(fat, i);
        if(p == NULL) {
            return -1;
        }

        if(p->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        if(p->DIR_Name[0] == DIRITEM_NAME_FREE) {
            continue;
        }

        if(diritem_name_match(p, filename)) {
            cluster_t cluster = (p->DIR_FstClusHI << 16) | (p->DIR_FstClusLO);
            cluster_free_chain(fat, cluster);
            
            kernel_memset(p, 0, sizeof(diritem_t));
            return write_dir_entry(fat, p, i);
        }
    }
    return -1;
}

int fat16fs_open(struct _fs_t* fs, const char* filepath, file_t* file) {
    fat16_t* fat = (fat16_t*)fs->data;
    
    diritem_t* item = NULL;
    int index = -1;

    for(int i = 0; i < fat->root_entry_cnt; i++) {
        diritem_t* p = read_dir_entry(fat, i);
        if(p == NULL) {
            return -1;
        }

        if(p->DIR_Name[0] == DIRITEM_NAME_END) {
            index = i;
            break;
        }

        if(p->DIR_Name[0] == DIRITEM_NAME_FREE) {
            index = i;
            continue;
        }

        if(diritem_name_match(p, filepath)) {
            item = p;
            index = i;
            break;
        }
    }

    if(item != NULL) {
        read_from_diritem(fat, file, item, index);
        return 0;
    }
    else if((file->mode & O_CREAT) && (index >= 0)) { // 若item为空, 且需要创建该文件
        diritem_t itm;
        diritem_init(&itm, filepath, 0);

        int ret = write_dir_entry(fat, &itm, index);
        if(ret < 0) {
            log_print("create file failed.");
            return -1;
        }
        read_from_diritem(fat, file, item, index);
        return 0;
    }

    return -1;
}

int fat16fs_read(char* buf, int size, file_t* file) {
    fat16_t* fat = (fat16_t*)file->fs->data;

    // 调整读取量, 不要超过文件总量
    uint32_t nbytes = size;
    if(file->pos + nbytes > file->size) {
        nbytes = file->size - file->pos;
    }

    uint32_t len = 0;   // 最终读取总字节数

    // 逐字节读取
    while(nbytes > 0) {
        // 每轮能够读取的长度, 会不断调整, 最终就是每轮读取了的量
        uint32_t curr_read = nbytes;

        // 当前位置在簇中的偏移
        uint32_t cluster_offset = file->pos % fat->cluster_bytes_size;

        // 当前位置的所处扇区
        uint32_t start_sector = fat->data_start + (file->cblk - 2) * fat->sectors_per_cluster; // 簇号从2开始

        // 若是整簇读取
        if(cluster_offset == 0 && nbytes == fat->cluster_bytes_size) {
            int ret = dev_read(fat->fs->dev_id, start_sector, buf, fat->sectors_per_cluster);
            if(ret < 0) {
                return len;
            }
            curr_read = fat->cluster_bytes_size;
        }
        // 若不是整簇读取
        else {

            // 若跨簇, 则只读取第一个簇
            if(cluster_offset + curr_read > fat->cluster_bytes_size) {
                curr_read = fat->cluster_bytes_size - cluster_offset;
            }

            fat->last_sector = -1;
            int ret = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sectors_per_cluster);
            if(ret < 0) {
                return len;
            }
            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);
        }

        buf += curr_read;
        nbytes -= curr_read;
        len += curr_read;

        int ret = move_file_pos(file, fat, curr_read, 0);
        if(ret < 0) {
            return len;
        }
    }
    return len;
}

int fat16fs_write(char* buf, int size, file_t* file) {
    return 0;
}

void fat16fs_close(file_t* file) {

}

int fat16fs_seek(file_t* file, uint32_t offset, int dir) {
    // file->pos <= offset
    if(dir != 0) {
        return -1;
    }

    fat16_t* fat = (fat16_t*)file->fs->data;
    cluster_t curr_clus = file->cblk;
    uint32_t curr_pos = 0;
    uint32_t off_to_move = offset;

    while(off_to_move) {
        uint32_t cur_offset = curr_pos % fat->cluster_bytes_size;
        uint32_t curr_move = off_to_move;

        if(cur_offset + curr_move < fat->cluster_bytes_size) {
            curr_pos += curr_move;
            break;
        }

        curr_move = fat->cluster_bytes_size - cur_offset;
        curr_pos += curr_move;
        off_to_move -= curr_move;

        curr_clus = cluster_get_next(fat, curr_clus);
        if(!cluster_is_valid(curr_clus)) {
            return -1;
        }
    }
    file->pos = curr_pos;
    file->cblk = curr_clus;
    return 0;
}

int fat16fs_stat(file_t* file, struct stat* st) {
    return -1;
}

int fat16fs_opendir(struct _fs_t* fs, const char* name, DIR* dir) {
    dir->index = 0;
    return 0;
}

int fat16fs_readdir(struct _fs_t* fs, DIR* dir, struct dirent* dirent) {
    fat16_t* fat = (fat16_t*)fs->data;

    // 遍历所有目录项
    while(dir->index < fat->root_entry_cnt) {
        // 读取目录项
        diritem_t* item = read_dir_entry(fat, dir->index);
        if(item == NULL) {
            return -1;
        }

        // 若当前目录项是空闲的并且后面没有目录项, 那么直接退出
        if(item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 当前目录项是空闲的, 但是后面还有目录项
        if(item->DIR_Name[0] != DIRITEM_NAME_FREE) {
            // 解析目录项类型
            file_type_t type = diritem_parse_type(item);
            if(type == FILE_NORMAL || type == FILE_DIR) {
                // 解析name到dirent->name中
                diritem_parse_name(item, dirent->name);
                dirent->size = item->DIR_FileSize;
                dirent->type = type;
                dirent->index = dir->index++;
                return 0;
            }
        }
        dir->index++;
    }
    return -1;
}

int fat16fs_closedir(struct _fs_t* fs, DIR* dir) {
    return 0;
}

static diritem_t* read_dir_entry(fat16_t* fat, int index) {
    if(index < 0 || index >= fat->root_entry_cnt) {
        return NULL;
    }

    int offset = index * sizeof(diritem_t);
    int sector = fat->root_start + offset / fat->bytes_per_sector;
    
    // 从缓冲中读取扇区
    int ret = bread_sector(fat, sector);
    if(ret < 0) {
        return NULL;
    }

    return (diritem_t*)(fat->fat_buffer + offset % fat->bytes_per_sector);
}

static int write_dir_entry(fat16_t* fat, diritem_t* dir, int index) {
    if(index < 0 || index >= fat->root_entry_cnt) {
        return -1;
    }

    int offset = index * sizeof(diritem_t);
    int sector = fat->root_start + offset / fat->bytes_per_sector;
    
    // 从缓冲中读取扇区
    int ret = bread_sector(fat, sector);
    if(ret < 0) {
        return -1;
    }

    kernel_memcpy(fat->fat_buffer + offset % fat->bytes_per_sector, dir, sizeof(diritem_t));

    return bwrite_sector(fat, sector);
}

static file_type_t diritem_parse_type(diritem_t* item) {
    if(item->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }
    if((item->DIR_Attr & DIRITEM_ATTR_LONG_NAME) == DIRITEM_ATTR_LONG_NAME) {
        return FILE_UNKNOWN;
    }
    return item->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
}

// file.c ===> FILE____C__ (_是空格)
static void diritem_parse_name(diritem_t* item, char* dest) {
    char* c = dest;
    kernel_memset(dest, 0, SFN_LEN + 1);
    
    char* ext = NULL;
    for(int i = 0; i < 11; i++) {
        if(item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }
        if(i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    if(ext && ext[1] == '\0') {
        ext[0] = '\0';
    }
}

static int bread_sector(fat16_t* fat, int sector) {
    if(sector == fat->last_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    if(cnt == 1) {
        fat->last_sector= sector;
        return 0;
    }
    return -1;
}

static int bwrite_sector(fat16_t* fat, int sector) {
    int cnt = dev_write(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    return (cnt == 1) ? 0 : -1;
}

static void read_from_diritem(fat16_t* fat, file_t* file, diritem_t* item, int index) {
    file->type = diritem_parse_type(item);
    file->size = item->DIR_FileSize;
    file->pos = 0;
    file->p_index = index;
    file->sblk = (item->DIR_FstClusHI << 16) | (item->DIR_FstClusLO);
    file->cblk = file->sblk;
}

static int diritem_init(diritem_t* item, const char* name, uint8_t attr) {
    to_sfn((char*)item->DIR_Name, name);
    item->DIR_FstClusHI = (uint16_t)(FAT_CLUSTER_INVALID >> 16);
    item->DIR_FstClusLO = (uint16_t)(FAT_CLUSTER_INVALID & 0xFFFF);
    item->DIR_FileSize = 0;
    item->DIR_Attr = attr;
    item->DIR_NTRes = 0;
    item->DIR_CrtTime= 0;
    item->DIR_CrtDate = 0;
    item->DIR_WrtTime = 0;
    item->DIR_WrtDate = 0;
    item->DIR_LastAccDate = 0;
    return 0;
}

static int diritem_name_match(diritem_t* item, const char* name) {
    char buf[11];
    to_sfn(buf, name);
    return kernel_memcmp(buf, item->DIR_Name, 11) == 0;
}

static void to_sfn(char* dest, const char* src) {
    kernel_memset(dest, ' ', 11);
    
    char* p = dest;
    char* end = dest + 11;
    while(*src && p < end) {
        char c = *src++;
        switch (c)
        {
        case '.':
            p = dest + 8;
            break;

        default:
            if(c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }
            *p++ = c;
            break;
        }
    }
}

static int move_file_pos(file_t* file, fat16_t* fat, uint32_t move_bytes, int expand) {
    // 当前位置在簇中的偏移量
    uint32_t c_offset = file->pos % fat->cluster_bytes_size;
    
    // 若pos将在下一个簇中, 那么调整pos到下一簇
    if(c_offset + move_bytes >= fat->cluster_bytes_size) {
        cluster_t next = cluster_get_next(fat, file->cblk);
        if(next == FAT_CLUSTER_INVALID) {
            return -1;
        }
        file->cblk = next;
    }
    file->pos += move_bytes;
    return 0;
}

static cluster_t cluster_get_next(fat16_t* fat, cluster_t curr) {
    // 若curr无效
    if(!cluster_is_valid(curr)) {
        return FAT_CLUSTER_INVALID;
    }

    // fat表中的扇区号和在扇区中的偏移
    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sector;
    int off_secter = offset % fat->bytes_per_sector;
    if(sector >= fat->fat_tbl_sectors) {
        log_print("cluster too big: %d", curr);
        return FAT_CLUSTER_INVALID;
    }

    int ret = bread_sector(fat, fat->fat_tbl_start + sector);
    if(ret < 0) {
        return FAT_CLUSTER_INVALID;
    }

    return *(cluster_t*)(fat->fat_buffer + off_secter);
}

static int cluster_set_next(fat16_t* fat, cluster_t curr, cluster_t next) {
    // 若curr无效
    if(!cluster_is_valid(curr)) {
        return -1;
    }

    // fat表中的扇区号和在扇区中的偏移
    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sector;
    int off_secter = offset % fat->bytes_per_sector;
    
    if(sector >= fat->fat_tbl_sectors) {
        log_print("cluster too big: %d", curr);
        return -1;
    }

    int ret = bread_sector(fat, fat->fat_tbl_start + sector);
    if(ret < 0) {
        return -1;
    }

    *(cluster_t*)(fat->fat_buffer + off_secter) = next;
    for(int i = 0; i < fat->fat_tbl_cnt; i++) {
        ret = bwrite_sector(fat, fat->fat_tbl_start + sector);
        if(ret < 0) {
            log_print("write cluster failed.");
            return -1;
        }
        sector += fat->fat_tbl_sectors;
    }
    return 0; 
}

static void cluster_free_chain(fat16_t* fat, cluster_t begin) {
    while (cluster_is_valid(begin)) {
        cluster_t next = cluster_get_next(fat, begin);

        cluster_set_next(fat, begin, FAT_CLUSTER_FREE);
        begin = next;
    }
}