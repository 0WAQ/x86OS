/***
 * 
 * 位图基本类型
 * 
 */
#ifndef BITMAP_T_H
#define BITMAP_T_H

#include "common/types.h"

typedef struct _bitmap_t {
    uint8_t* bits;          // 字节数组
    uint32_t bit_count;     // 表示位图表示的总位数
}bitmap_t;

#endif // BITMAP_T_H