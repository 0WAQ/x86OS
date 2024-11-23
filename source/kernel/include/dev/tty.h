/**
 * 
 * tty 头文件
 * 
 */
#ifndef TTY_H
#define TTY_H

#include "tty_t.h"
#include "ipc/sem_t.h"

/**
 * @brief 初始化tty的缓冲区
 */
void tty_fifo_init(tty_fifo_t* ofifo, char* buf, int len);

/**
 * @brief 打开tty设备
 */
int tty_open(device_t* dev);

/**
 * @brief 从tty读取数据
 */
int tty_read(device_t* dev, int addr , char* buf , int size);

/**
 * @brief 向tty写入数据
 */
int tty_write(device_t* dev, int addr , char* buf , int size);

/**
 * @brief 向tty发送命令
 */
int tty_control(device_t* dev, int cmd, int arg0, int arg1);

/**
 * @brief 关闭tty设备
 */
void tty_close(device_t* dev);

/**
 * @brief 获取dev对应的tty, 并判断tty是否有效
 */
static tty_t* get_tty(device_t* dev);

/**
 * @brief 向tty缓冲区写数据
 */
int tty_fifo_put(tty_fifo_t* fifo, char ch);

/**
 * @brief 从tty缓冲区读数据
 */
int tty_fifo_get(tty_fifo_t* fifo, char* ch);

/**
 * @brief 键盘通过该函数, 将要打印的字符交给tty_tbl[idx]
 */
void tty_in(int idx, char ch);

#endif // TTY_H