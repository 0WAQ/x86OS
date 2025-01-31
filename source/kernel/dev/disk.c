/**
 * 
 * 磁盘驱动c文件
 * 磁盘依次从sda,sdb,sdc开始编号，分区则从0开始递增
 * 其中0对应的分区信息为整个磁盘的信息
 * 
 */
#include "cpu/irq.h"
#include "dev/dev.h"
#include "dev/disk.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/boot_info.h"

/**
 * @brief disk的设备层回调函数
 */
dev_desc_t dev_disk_desc = {
    .name = "disk",
    .major = DEV_DISK,
    .open = disk_open,
    .read = disk_read,
    .write = disk_write,
    .control = disk_control,
    .close = disk_close
};

static disk_t disk_table[DISK_CNT];     // 磁盘表
static mutex_t mtx;
static sem_t sem;
static int task_no_op;

void disk_init() {
    log_print("Check disk...");
    
    kernel_memset(disk_table, 0, sizeof(disk_table));
    
    mutex_init(&mtx);
    sem_init(&sem, 0);
    
    // 检测各个硬盘, 读取相关信息
    for(int i = 0; i < DISK_PER_CHANNEL; i++) {
        disk_t* disk = disk_table + i;

        // sda, sdb, sdc, sdd
        kernel_sprintf(disk->name, "sd%c", i + 'a');
        disk->drive = (i == 0 ? DISK_MASTER : DISK_SLAVE);
        disk->port_base = DISK_IOBASE_PRIMARY;

        disk->mtx = &mtx;
        disk->sem = &sem;

        int ret = identify_disk(disk);
        if(ret == 0) {
            print_disk_info(disk);
        }
    }
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
    u16_t buf[256];
    ata_read_data(disk, buf, sizeof(buf));
    disk->sector_count = *(u32_t *)(buf + 100);
    disk->sector_size = SECTOR_SIZE;            // 固定为512字节大小

    // 分区0保存了整个磁盘的信息
    partinfo_t* part = disk->partinfo + 0;
    part->disk = disk;
    kernel_sprintf(part->name, "%s%d", disk->name, 0);  // sdx0
    part->start_sector = 0;
    part->total_sector = disk->sector_count;
    part->type = FS_INVALID;

    // 接下来识别硬盘上的分区信息
    detect_part_info(disk);
    return 0;
}

static void print_disk_info(disk_t* disk) {
    log_print("%s:", disk->name);
    log_print("  port_base: %x", disk->port_base);
    log_print("  total_size: %d m", disk->sector_count * disk->sector_size / 1024 /1024);
    for(int i = 0; i < DISK_PRIMARY_PART_CNT; i++) {
        partinfo_t* part_info = disk->partinfo + i;
        if(part_info->type != FS_INVALID) {
            log_print("     %s  type: %x, start_sector: %d, count: %d", 
                                part_info->name, part_info->type, 
                                part_info->start_sector, part_info->total_sector);
        }
    }
}

static int detect_part_info(disk_t* disk) {
    mbr_t mbr;

    // 读取mbr区
    ata_send_cmd(disk, 0, 1, DISK_CMD_READ);
    int err = ata_wait_data(disk);
    if(err < 0) {
        log_print("read mbr failed.");
        return err;
    }
    ata_read_data(disk, &mbr, sizeof(mbr_t));

    // 遍历4个主分区描述，不考虑支持扩展分区
    part_item_t* item = mbr.part_time;
    partinfo_t* part_info = disk->partinfo + 1;
    for(int i = 0; i < MBR_PRIMARY_PART_NR; i++, item++, part_info++) {
        part_info->type = item->system_id;

        // 没有分区，清空part_info
        if(part_info->type == FS_INVALID) {
            part_info->total_sector = 0;
            part_info->start_sector = 0;
            part_info->disk = NULL;
        }
        else {
            // 在主分区中找到，复制信息
            kernel_sprintf(part_info->name, "%s%d", disk->name, i + 1);  // sdx0
            part_info->start_sector = item->relative_sectors;
            part_info->total_sector = disk->sector_count;
            part_info->disk = disk;
        }
    }
}

static int disk_open(device_t* dev) {
    // 0xa0 -- 磁盘编号: a, b, c 
    //      -- 分区号: 0, 1, 2
    int disk_idx = (dev->minor >> 4) - 0xa;
    int part_idx = (dev->minor & 0xf);

    if(disk_idx >= DISK_CNT || part_idx >= DISK_PRIMARY_PART_CNT) {
        log_print("device minor error: %d", dev->minor);
        return -1;
    }

    disk_t* disk = disk_table + disk_idx;
    if(disk->sector_size == 0) {
        log_print("disk doesn't exist, device: sd%x", dev->minor);
        return -1;
    }

    partinfo_t* part_info = disk->partinfo + part_idx;
    if(part_info->total_sector == 0) {
        log_print("part dosen't exist, device: sd%x", dev->minor);
        return -1;
    }

    dev->data = part_info;

    // 安装并打开中断
    irq_install(IRQ14_HARDDISK_PRIMARY, GATE_ATTR_DPL0, exception_handler_ide_primary);
    irq_enable(IRQ14_HARDDISK_PRIMARY);
    return 0;
}

static int disk_read(device_t* dev, int start_sector, char* buf, int count) {
    // 获取分区信息
    partinfo_t* part_info = (partinfo_t*)dev->data;
    if(part_info == NULL) {
        log_print("Get part info failed, device: sd%x", dev->minor);
        return -1;
    }

    disk_t* disk = part_info->disk;
    if(disk == NULL) {
        log_print("No disk, device: sd%x", dev->minor);
        return -1;
    }

    // 互斥锁, 两个主从硬盘都是通过同一端口号进行操作的
    mutex_lock(disk->mtx);
    task_no_op = 1;

    // 发送命令
    ata_send_cmd(disk, part_info->start_sector + start_sector, count, DISK_CMD_READ);

    int cnt;
    for(cnt = 0; cnt < count; cnt++, buf += disk->sector_size) {
        // 等待硬盘的中断通知
        if(get_curr_task() != NULL) {
            sem_wait(disk->sem);
        }

        // 实际上在ata_wait_data中并不会等待
        int err = ata_wait_data(disk);
        if(err < 0) {
            log_print("disk(%s), read error: start_sector: %d, count: %d", 
                        disk->name, start_sector, count);
            break;
        }

        // 读取数据
        ata_read_data(disk, buf, disk->sector_size);
    }
    mutex_unlock(disk->mtx);
    return cnt;
}

static int disk_write(device_t* dev, int start_sector, char* buf, int count) {
    // 获取分区信息
    partinfo_t* part_info = (partinfo_t*)dev->data;
    if(part_info == NULL) {
        log_print("Get part info failed, device: sd%x", dev->minor);
        return -1;
    }

    disk_t* disk = part_info->disk;
    if(disk == NULL) {
        log_print("No disk, device: sd%x", dev->minor);
        return -1;
    }

    // 互斥锁, 两个主从硬盘都是通过同一端口号进行操作的
    mutex_lock(disk->mtx);
    task_no_op = 1;

    // 发送命令
    ata_send_cmd(disk, part_info->start_sector + start_sector, count, DISK_CMD_WRITE);

    int cnt;
    for(cnt = 0; cnt < count; cnt++, buf += disk->sector_size) {

        // 先写数据
        ata_write_data(disk, buf, disk->sector_size);
        
        // 等待, 直到写完成
        if(get_curr_task() != NULL) {
            sem_wait(disk->sem);
        }

        int err = ata_wait_data(disk);
        if(err < 0) {
            log_print("disk(%s), write error: start_sector: %d, count: %d", 
                        disk->name, start_sector, count);
            break;
        }
    }
    mutex_unlock(disk->mtx);
    return cnt;
}

static int disk_control(device_t* dev, int cmd, int arg0, int arg1) {
    return -1;
}

static void disk_close(device_t* dev) {

}

void do_handler_ide_primary(exception_frame_t* frame) {
    pic_send_eoi(IRQ14_HARDDISK_PRIMARY);
    if(task_no_op && get_curr_task() != NULL)  {
        sem_notify(&sem);
    }
}