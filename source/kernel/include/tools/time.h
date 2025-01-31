/**
 * 
 * 时间相关头文件
 * 
 */
#ifndef TIME_H
#define TIME_H

#include "time_t.h"
#include "common/cpu_instr.h"
#include <time.h>

// 从CMOS RAM中读取时间
#define CMOS_READ(addr) ({outb_p(0x80|addr, 0x70); inb_p(0x71);})

// 将读取出来的时间按照BCD解码
#define BCD_TO_BIN(val) ( (val) = ((val) & 0x0F) + ((val) >> 4) * 10)

/**
 * @brief 获取当前时间的struct tm结构
 */
int get_tm(struct tm* tm);

/**
 * @brief 将tm结构体转换为从1970-1-1开始的秒数
 */
time_t kernel_mktime(struct tm* tm);

#endif // TIME_H