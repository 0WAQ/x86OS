/**
 * 
 * 文件系统C文件
 * 
 */

#include "fs/fs.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"

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

int sys_open(const char* filename, int flags, ...) {
    if(filename[0] == '/') {
        read_disk(5000, 80, TEMP_ADDR);
        temp_pos = TEMP_ADDR;
        return TEMP_FILE_ID;
    }

    return -1;
}

int sys_read(int fd, char* buf, int len) {
    if(fd == TEMP_FILE_ID) {
        kernel_memcpy(buf, temp_pos, len);
        temp_pos += len;
        return len;
    }

    return -1;
}

int sys_write(int fd, char* buf, int len) {
    return 0;
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