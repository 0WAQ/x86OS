/**
 * 
 * 磁盘驱动头文件
 * 
 */
#ifndef DISK_H
#define DISK_H

#include "disk_t.h"
#include "common/cpu_instr.h"

/**
 * @brief 磁盘初始化
 */
void disk_init();

/**
 * @brief 检测磁盘相关的信息
 */
static int identify_disk(disk_t* disk);

/**
 * @brief 打印磁盘信息
 */
static void print_disk_info(disk_t* disk);

/**
 * @brief 发送ATA命令, 支持最多16位的扇区
 */
static void ata_send_cmd(disk_t* disk, uint32_t start_sector, uint32_t sector_count, int cmd);

/**
 * @brief 检测分区表
 */
static int detect_part_info(disk_t* disk);

/**
 * @brief 读取ATA数据端口
 */
static inline void ata_read_data (disk_t* disk, void* buf, int size) {
    uint16_t* c = (uint16_t*)buf;
    for(int i = 0; i < size / 2; i++) {
        *c++ = inw(DISK_DATA(disk));
    }
}

/**
 * @brief 读取ATA数据端口
 */
static inline void ata_write_data (disk_t* disk, void* buf, int size) {
    uint16_t* c = (uint16_t*)buf;
    for(int i = 0; i < size / 2; i++) {
        outw(DISK_DATA(disk), *c++);
    }
}

/**
 * @brief 等待磁盘有数据到达
 */
static inline int ata_wait_data (disk_t* disk) {
    uint8_t status;
	while(1) {
        // 等待数据或者有错误
        status = inb(DISK_STATUS(disk));
        if ((status & (DISK_STATUS_BUSY | DISK_STATUS_DRQ | DISK_STATUS_ERR))
                        != DISK_STATUS_BUSY) {
            break;
        }
    }

    // 检查是否有错误
    return (status & DISK_STATUS_ERR) ? -1 : 0;
}

#endif // DISK_H