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
#include "common/cpu_instr.h"
#include <sys/stat.h>

void fs_init() {

    // 初始化文件表
    file_table_init();
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
        fd = 0;
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