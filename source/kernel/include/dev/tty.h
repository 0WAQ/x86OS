/**
 * 
 * tty 头文件
 * 
 */
#ifndef TTY_H
#define TTY_H

#include "tty_t.h"

/**
 * @brief
 */
void tty_fifo_init(tty_fifo_t* ofifo, char* buf, int len);

/**
 * @brief
 */
int tty_open(device_t* dev);

/**
 * @brief
 */
int tty_read(device_t* dev, int addr , char* buf , int size);

/**
 * @brief
 */
int tty_write(device_t* dev, int addr , char* buf , int size);

/**
 * @brief
 */
int tty_control(device_t* dev, int cmd, int arg0, int arg1);

/**
 * @brief
 */
void tty_close(device_t* dev);

#endif // TTY_H