/**
 * 
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * boot扇区容量较小，仅512字节。由于dbr占用了不少字节，导致其没有多少空间放代码，
 * 所以功能只能最简化,并且要开启最大的优化-os
 *
 */
__asm__(".code16gcc");

#include "boot.h"

// loader加载的地址
#define LOADER_START_ADDR   0x8000

/**
 * Boot的C入口函数
 * 只完成一项功能，即从磁盘找到loader文件然后加载到内容中，并跳转过去
 */
void boot_entry() {
    // 将0x8000这个指针转换为 void (*)(), 并调用
    ((void (*)())LOADER_START_ADDR)();
} 

