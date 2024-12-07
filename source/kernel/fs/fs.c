/**
 * 
 * 文件系统C文件
 * 
 */

#include "fs/fs.h"
#include "fs/file.h"
#include "dev/dev.h"
#include "dev/console.h"
#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/list.h"
#include "common/cpu_instr.h"
#include <sys/stat.h>

static list_t mounted_list;             // 挂载链表
static list_t free_list;                // 空闲链表
static fs_t fs_table[FS_TABLE_SIZE];    // fs表
extern fs_op_t devfs_op;                // TODO: 临时的

void fs_init() {

    // 初始化文件系统的链表
    mount_list_init();

    // 初始化文件表
    file_table_init();

    // 挂载devfs
    fs_t* fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != NULL);
}

// TODO: 临时使用
void read_disk(uint32_t sector, uint32_t sector_cnt, uint8_t* buffer) {
    
    // 读取LBA参数
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_cnt >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_cnt));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

    // 读取数据
    uint16_t* buf = (uint16_t*)buffer;
    while(sector_cnt-- > 0) {

        // 读前检查，等待数据就位
        while((inb(0x1F7) & 0x88) != 0x08);

        // 读取数据并将其写入到缓存中
        for(int i = 0; i < 512 / 2; i++) {
            *buf++ = inw(0x1F0);
        }
    }
}

// TODO: TEMP
static int is_path_valid(const char* path) {
    if((path == NULL) || (path[0] == '\0')) {
        return 0;
    }
    return 1;
}

int sys_open(const char* filename, int flags, ...) {

    if(kernel_strncmp(filename, "tty", 3) == 0) {
        if(!is_path_valid(filename)) {
            log_print("path is not valid.");
            return -1;
        }

        int fd = -1;
        file_t* file = file_alloc();
        if(file) {
            fd = task_alloc_fd(file);
            if(fd < 0) {
                goto sys_open_failed;
            }
        }
        else {
            goto sys_open_failed;
        }

        if(kernel_strlen(filename) < 5) {
            goto sys_open_failed;
        }
        int idx = filename[4] - '0';
        int dev_id = dev_open(DEV_TTY, idx, 0);
        if(dev_id < 0) {
            goto sys_open_failed;
        }

        file->dev_id = dev_id;
        file->mode = 0;
        file->pos = 0;
        file->ref = 1;
        file->type = FILE_TTY;

        kernel_strncpy(file->filename, filename, FILENAME_SIZE);
        return fd;

sys_open_failed:
        if(file) {
            file_free(file);
        }

        if(fd >= 0) {
            task_remove_fd(fd);
        }
    }
    else
    {
        // TODO:
        if(filename[0] == '/') {
            read_disk(5000, 80, (uint8_t*)TEMP_ADDR);
            temp_pos = (uint8_t*)TEMP_ADDR;
            return TEMP_FILE_ID;
        }
    }
    return -1;
}

int sys_read(int fd, char* buf, int len) {
    // TODO: 
    if(fd == TEMP_FILE_ID) {
        kernel_memcpy(buf, temp_pos, len);
        temp_pos += len;
        return len;
    } else {
        file_t* p = get_task_file(fd);
        if(!p) {
            log_print("file not opened.");
            return -1;
        }
        return dev_read(p->dev_id, 0, buf, len);
    }

    return -1;
}

int sys_write(int fd, char* buf, int len) {
    fd = 0;
    file_t* p = get_task_file(fd);
    if(!p) {
        log_print("file not opened.");
        return -1;
    }
    return dev_write(p->dev_id, 0, buf, len);
}

int sys_lseek(int fd, int offset, int dir) {
    if(fd == TEMP_FILE_ID) {
        temp_pos = (uint8_t*)(TEMP_ADDR + offset);
        return 0;
    }

    return -1;
}

int sys_close(int fd) {
    return 0;
}

int sys_isatty(int fd) {
    return -1;
}

int sys_fstat(int fd, struct stat* st) {
    return -1;
}

int sys_dup(int fd) {
    if((fd < 0) || (fd >= TASK_OFILE_NR)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(!p) {
        log_print("file not opened.");
        return -1;
    }

    // 新创建一个fd与p相同
    int new = task_alloc_fd(p);
    if(new < 0) {
        log_print("no task file avaliable.");
        return -1;
    }
    
    p->ref++;
    return new;
}

static void mount_list_init() {
    
    // 初始化空闲链表
    list_init(&free_list);
    for(int i = 0; i < FS_TABLE_SIZE; i++) {
        list_insert_first(&free_list, &fs_table[i].node);
    }

    // 初始化挂载链表
    list_init(&mounted_list);
}

static fs_op_t* get_fs_op(fs_type_t type, int major) {
    switch (type)
    {
    case FS_DEVFS:
        return &(devfs_op);
    default:
        return NULL;
    }
    return NULL;
}

static fs_t* mount(fs_type_t type, char* mount_point, int dev_major, int dev_minor) {
    fs_t* fs = NULL;
    log_print("mount file system, name: %s, dev: %x,", mount_point, dev_major);

    // 判断当前文件是否挂载过
    list_node_t* curr = list_first(&mounted_list);      // 遍历挂载链表
    while(curr != NULL) {
        fs_t* p = list_entry_of(curr, fs_t, node);
        if(kernel_strncmp(fs->mount_point, mount_point, FS_MOUNTP_SIZE) == 0) {
            log_print("fs already mounted.");
            goto mount_failed;
        }
        curr = list_node_next(curr);
    }

    // 取出空闲链表的一个节点, 将其分配给fs
    list_node_t* free_node = list_remove_first(&free_list);
    if(free_node == NULL) {
        log_print("no free fs, mount failed.");
        goto mount_failed;
    }
    fs = list_entry_of(free_node, fs_t, node);

    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strncpy(fs->mount_point, mount_point, FS_MOUNTP_SIZE);
    // fs->node = free_node;

    // 获取当前fs的操作函数
    fs_op_t* op = get_fs_op(type, dev_major);
    if(op == NULL) {
        log_print("unsupport fs type: %d", type);
        goto mount_failed;
    }
    fs->op = op;

    // 执行挂载
    if(op->mount(fs, dev_major, dev_minor) < 0) {
        log_print("mount fs %s failed.", mount_point);
        goto mount_failed;
    }

    // 将节点移至挂载链表中
    list_insert_last(&mounted_list, &fs->node);
    return fs;

mount_failed:
    if(fs) {
        list_insert_last(&free_list, &fs->node);
    }

    return NULL;
}