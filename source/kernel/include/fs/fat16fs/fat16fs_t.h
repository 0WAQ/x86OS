/**
 * 
 * FAT16文件系统基本类型
 * 
 */
#ifndef FAT16FS_T_H
#define FAT16FS_T_H

#include "common/types.h"

/**
 * @brief DBR类型
 */
#pragma pack(1)
typedef struct _dbr_t {
    uint8_t  BS_jmpBoot[3];                 // 跳转代码
    uint8_t  BS_OEMName[8];                 // OEM名称
    uint16_t BPB_BytsPerSec;                // 每扇区字节数
    uint8_t  BPB_SecPerClus;                // 每簇扇区数
    uint16_t BPB_RsvdSecCnt;                // 保留区扇区数
    uint8_t  BPB_NumFATs;                   // FAT表项数
    uint16_t BPB_RootEntCnt;                // 根目录项目数
    uint16_t BPB_TotSec16;                  // 总的扇区数
    uint8_t  BPB_Media;                     // 媒体类型
    uint16_t BPB_FATSz16;                   // FAT表项大小
    uint16_t BPB_SecPerTrk;                 // 每磁道扇区数
    uint16_t BPB_NumHeads;                  // 磁头数
    uint32_t BPB_HiddSec;                   // 隐藏扇区数
    uint32_t BPB_TotSec32;                  // 总的扇区数

	uint8_t  BS_DrvNum;                     // 磁盘驱动器参数
	uint8_t  BS_Reserved1;				    // 保留字节
	uint8_t  BS_BootSig;                    // 扩展引导标记
	uint32_t BS_VolID;                      // 卷标序号
	uint8_t  BS_VolLab[11];                 // 磁盘卷标
	uint8_t  BS_FileSysType[8];             // 文件类型名称
} dbr_t;
#pragma pack()

/**
 * @brief FAT结构
 */
typedef struct _fat16_t {
    // fat文件系统本身信息
    uint32_t fat_tbl_start;             // FAT表起始地址
    uint32_t fat_tbl_cnt;               // FAT表数量
    uint32_t fat_tbl_sectors;           // 每FAT表扇区数
    uint32_t bytes_per_sectors;         // 每扇区字节数
    uint32_t sectors_per_clusters;      // 每簇扇区数
    uint32_t root_start;                // 根目录扇区数
    uint32_t root_entry_cnt;            // 根目录的项数
    uint32_t data_start;                // 文件数据起始扇区
    uint32_t clusters_byte_size;        // 一个簇的字节大小

    // 与文件系统读写相关信息
    uint8_t* fat_buffer;                // FAT表项缓冲
    struct _fs_t* fs;                   // 所在的文件系统

}fat16_t;

#endif // FAT16FS_T_H