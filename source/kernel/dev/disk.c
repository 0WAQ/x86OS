/**
 * 
 * 磁盘驱动c文件
 * 磁盘依次从sda,sdb,sdc开始编号，分区则从0开始递增
 * 其中0对应的分区信息为整个磁盘的信息
 * 
 */
#include "dev/disk.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/boot_info.h"

/**
 * @brief 磁盘表
 */
static disk_t disk_table[DISK_CNT];

void disk_init() {
    log_print("Check disk...");
    
    kernel_memset(disk_table, 0, sizeof(disk_table));
    for(int i = 0; i < DISK_PER_CHANNEL; i++) {
        disk_t* disk = disk_table + i;

        // sda, sdb, sdc, sdd
        kernel_sprintf(disk->name, "sd%c", i + 'a');
        disk->drive = (i == 0 ? DISK_MASTER : DISK_SLAVE);
        disk->port_base = DISK_IOBASE_PRIMARY;

        int ret = identify_disk(disk);
        if(ret == 0) {
            print_disk_info(disk);
        }
    }
}

static void ata_send_cmd(disk_t* disk, uint32_t start_sector, uint32_t sector_count, int cmd) {
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

static int identify_disk(disk_t* disk) {
    ata_send_cmd(disk, 0, 0, DISK_CMD_IDENTIFY);

    // 检测状态，如果为0，则控制器不存在
    int err = inb(DISK_STATUS(disk));
    if (err == 0) {
        log_print("%s doesn't exist\n", disk->name);
        return -1;
    }

    // 等待数据就绪, 此时中断还未开启，因此暂时可以使用查询模式
    err = ata_wait_data(disk);
    if (err < 0) {
        log_print("disk[%s]: read failed!\n", disk->name);
        return err;
    }

    // 读取返回的数据，特别是uint16_t 100 through 103
    // 测试用的盘： 总共102400 = 0x19000， 实测会多一个扇区，为vhd磁盘格式增加的一个扇区
    uint16_t buf[256];
    ata_read_data(disk, buf, sizeof(buf));
    disk->sector_count = *(uint32_t *)(buf + 100);
    disk->sector_size = SECTOR_SIZE;            // 固定为512字节大小
    return 0;
}

static void print_disk_info(disk_t* disk) {
    log_print("%s:", disk->name);
    log_print("  port_base: %x", disk->port_base);
    log_print("  total_size: %d m", disk->sector_count * disk->sector_size / 1024 /1024);
}