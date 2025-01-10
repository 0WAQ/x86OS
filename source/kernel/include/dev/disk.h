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
 * @brief 检测分区表
 */
static int detect_part_info(disk_t* disk);

/**
 * @brief 打开disk设备
 */
int disk_open(device_t* dev);

/**
 * @brief 从disk读取数据
 */
int disk_read(device_t* dev, int start_sector, char* buf, int count);

/**
 * @brief 向disk写入数据
 */
int disk_write(device_t* dev, int start_sector, char* buf, int count);

/**
 * @brief 向disk发出命令
 */
int disk_control(device_t* dev, int cmd, int arg0, int arg1);

/**
 * @brief 关闭disk设备
 */
void disk_close(device_t* dev);

/**
 * @brief 发送ATA命令, 支持最多16位的扇区
 */
static inline void ata_send_cmd(disk_t* disk, uint32_t start_sector, uint32_t sector_count, int cmd) {
    outb(DISK_DRIVE(disk), DISK_DRIVE_BASE | disk->drive);		    // 使用LBA寻址，并设置驱动器

	// 必须先写高字节
	outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count >> 8));	// 扇区数高8位
	outb(DISK_LBA_LOW(disk), (uint8_t) (start_sector >> 24));		// LBA参数的24~31位
	outb(DISK_LBA_MID(disk), 0);									// 高于32位不支持
	outb(DISK_LBA_HIGH(disk), 0);								    // 高于32位不支持
	outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count));		// 扇区数量低8位
	outb(DISK_LBA_LOW(disk), (uint8_t) (start_sector >> 0));		// LBA参数的0-7
	outb(DISK_LBA_MID(disk), (uint8_t) (start_sector >> 8));		// LBA参数的8-15位
	outb(DISK_LBA_HIGH(disk), (uint8_t) (start_sector >> 16));		// LBA参数的16-23位

	// 选择对应的主-从磁盘
	outb(DISK_CMD(disk), (uint8_t)cmd);
}

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