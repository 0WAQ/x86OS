/**
 * 
 * 设备描述头文件
 * 
 */
#ifndef DEV_H
#define DEV_H

#include "dev_t.h"

/**
 * @brief
 */
int dev_open(int major, int minor, void* data);

/**
 * @brief
 */
int dev_read(int dev_id, int addr, char* buf, int size);

/**
 * @brief
 */
int dev_write(int dev_id, int addr, char* buf, int size);

/**
 * @brief
 */
int dev_control(int dev_id, int cmd, int arg0, int arg1);

/**
 * @brief
 */
void dev_close(int dev_id);

/**
 * @brief
 */
static int is_devid_bad(int dev_id);

#endif // DEV_H
