/***
 * 
 * 位图相关头文件
 * 
 */
#ifndef BITMAP_H
#define BITMAP_H

#include "bitmap_t.h"

/**
 * @brief 初始化位图
 */
void bitmap_init(bitmap_t* bitmap, u8_t* bits, u32_t bit_count, int val);

/**
 * @brief 获取位图某一位的值 
 */
int bitmap_get_bit(bitmap_t* bitmap, u32_t index);

/**
 * @brief 设置位图连续一段的值
 */
void bitmap_set_bit(bitmap_t* bitmap, u32_t index, u32_t count, int val);

/**
 * @brief 判断某一位是否置1
 */
int bitmap_is_set(bitmap_t* bitmap, u32_t index);

/**
 * @brief 分配一段连续的值为val的位
 */
int bitmap_alloc_nbits(bitmap_t* bitmap, u32_t count, int val);

/**
 * @brief 计算位图中有多少个字节
 */
u32_t bitmap_byte_count(u32_t bit_count);

#endif // BITMAP_H