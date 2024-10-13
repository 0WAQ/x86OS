/**
 * 
 * 系统启动信息
 * 
 */
#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGION_MAX 10  // RAM区的最大数量

/**
 * @brief 启动信息参数
 */
typedef struct _boot_info_t {
    
    /**
     * @brief RAM区信息结构，用于存放内存中可用的RAM区域
     */
    struct {
        uint32_t start; // RAM区起始地址
        uint32_t size;  // RAM区大小
    }ram_region_cfg[BOOT_RAM_REGION_MAX];
    int ram_region_count;   // 内存中可用RAM区的个数

}boot_info_t;

// 磁盘一个扇区的大小
#define SECTOR_SIZE 512

// 内核映像文件加载的起始地址，1MB
#define SYS_KERNEL_LOAD_ADDR (0x100000)

#endif // BOOT_INFO_H 