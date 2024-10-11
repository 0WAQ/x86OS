/**
 * 
 * 32位loader的头文件
 * 
 */
#ifndef LOADER_32_H
#define LOADER_32_H

/**
 * @brief 二级引导器32位入口
 */
void loader_kernel();


/**
 * @brief 读磁盘，从磁盘上加载内核
 */
void read_disk(uint32_t sector, uint32_t sector_cnt, uint8_t* buffer);


#endif // LOADER_32_H