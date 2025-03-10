/**
 * 
 * FAT16文件系统基本类型
 * 
 */
#ifndef FAT16FS_T_H
#define FAT16FS_T_H

#include "common/types.h"
#include "ipc/mutex_t.h"

#define DIRITEM_NAME_FREE               0xE5                // 目录项空闲名标记
#define DIRITEM_NAME_END                0x00                // 目录项结束名标记

#define DIRITEM_ATTR_READ_ONLY          0x01                // 目录项属性：只读
#define DIRITEM_ATTR_HIDDEN             0x02                // 目录项属性：隐藏
#define DIRITEM_ATTR_SYSTEM             0x04                // 目录项属性：系统类型
#define DIRITEM_ATTR_VOLUME_ID          0x08                // 目录项属性：卷id
#define DIRITEM_ATTR_DIRECTORY          0x10                // 目录项属性：目录
#define DIRITEM_ATTR_ARCHIVE            0x20                // 目录项属性：归档
#define DIRITEM_ATTR_LONG_NAME          0x0F                // 目录项属性：长文件名

#define SFN_LEN                    	 	11                  // sfn文件名长

#define FAT_CLUSTER_INVALID             0xFFF8              // 无效簇号
#define FAT_CLUSTER_FREE                0x00                // 空闲簇号

/**
 * @brief 簇
 */
typedef u16_t cluster_t;

/**
 * @brief 目录项结构
 */
#pragma pack(1)
typedef struct _diritem_t {
    u8_t  DIR_Name[11];          // 文件名
    u8_t  DIR_Attr;              // 属性
    u8_t  DIR_NTRes;             // 保留位
    u8_t  DIR_CrtTimeTeenth;     // 创建时间的毫秒
    u16_t DIR_CrtTime;           // 创建时间
    u16_t DIR_CrtDate;           // 创建日期
    u16_t DIR_LastAccDate;       // 最后访问日期
    u16_t DIR_FstClusHI;         // 簇号高16位
    u16_t DIR_WrtTime;           // 修改时间
    u16_t DIR_WrtDate;           // 修改时期
    u16_t DIR_FstClusLO;         // 簇号低16位
    u32_t DIR_FileSize;          // 文件字节大小
}diritem_t;
#pragma pack()

/**
 * @brief DBR类型
 */
#pragma pack(1)
typedef struct _dbr_t {
    u8_t  BS_jmpBoot[3];                 // 跳转代码
    u8_t  BS_OEMName[8];                 // OEM名称
    u16_t BPB_BytsPerSec;                // 每扇区字节数
    u8_t  BPB_SecPerClus;                // 每簇扇区数
    u16_t BPB_RsvdSecCnt;                // 保留区扇区数
    u8_t  BPB_NumFATs;                   // FAT表项数
    u16_t BPB_RootEntCnt;                // 根目录项目数
    u16_t BPB_TotSec16;                  // 总的扇区数
    u8_t  BPB_Media;                     // 媒体类型
    u16_t BPB_FATSz16;                   // FAT表项大小
    u16_t BPB_SecPerTrk;                 // 每磁道扇区数
    u16_t BPB_NumHeads;                  // 磁头数
    u32_t BPB_HiddSec;                   // 隐藏扇区数
    u32_t BPB_TotSec32;                  // 总的扇区数

	u8_t  BS_DrvNum;                     // 磁盘驱动器参数
	u8_t  BS_Reserved1;				    // 保留字节
	u8_t  BS_BootSig;                    // 扩展引导标记
	u32_t BS_VolID;                      // 卷标序号
	u8_t  BS_VolLab[11];                 // 磁盘卷标
	u8_t  BS_FileSysType[8];             // 文件类型名称
} dbr_t;
#pragma pack()

/**
 * @brief FAT结构
 */
typedef struct _fat16_t {
    // fat文件系统本身信息
    u32_t fat_tbl_start;             // FAT表起始扇区号
    u32_t fat_tbl_cnt;               // FAT表数量
    u32_t fat_tbl_sectors;           // 每FAT表扇区数
    u32_t bytes_per_sector;          // 每扇区字节数
    u32_t sectors_per_cluster;       // 每簇扇区数
    u32_t root_start;                // 根目录扇区数
    u32_t root_entry_cnt;            // 根目录的项数
    u32_t data_start;                // 文件数据起始扇区
    u32_t cluster_bytes_size;        // 一个簇的字节大小

    // 与文件系统读写相关信息
    u8_t* fat_buffer;                // FAT表项缓冲
    int last_sector;                    // 上次读取的扇区号
    struct _fs_t* fs;                   // 所在的文件系统

    mutex_t mtx;

}fat16_t;

#endif // FAT16FS_T_H