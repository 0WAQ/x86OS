/**
 * 
 * tty 基本类型
 * 
 */
#ifndef TTY_T_H
#define TTY_T_H

#include "dev/dev_t.h"
#include "ipc/sem_t.h"

#define TTY_NR              (8)         // 最多支持的tty设备

#define TTY_OBUF_SIZE       (512)       // tty输出缓冲大小
#define TTY_IBUF_SIZE       (512)       // tty输入缓冲大侠奥

// 输出标志位的相关宏
#define TTY_OCRLF           (1 << 0)    // 开启后,打印会将 \n 转湖岸为 \r\n

// 输入标志位的相关宏
#define TTY_ICRLF           (1 << 0)    // 开启后,输入会将 \n 转换为 \r\n
#define TTY_IECHO           (1 << 1)    // 开启后,输入还会回显

// 控制tty的回显
#define TTY_CMD_ECHO        (0x1)

/**
 * @brief tty缓冲区
 */
typedef struct _tty_fifo_t {
    char* buf;
    int size;           // 缓冲区的大小
    int read, write;    // 当前读写位置
    int cnt;            // 缓冲区有效载荷
}tty_fifo_t;

/**
 * @brief tty结构
 */
typedef struct _tty_t {
    char obuf[TTY_OBUF_SIZE];
    char ibuf[TTY_IBUF_SIZE];
    tty_fifo_t ofifo;
    tty_fifo_t ififo;
    sem_t osem;                 // 输出相关信号量
    sem_t isem;                 // 输入相关信号量
    int oflags;                 // 输出标志位
    int iflags;                 // 输入标志位

    int console_index;          // 控制台索引号
}tty_t;

#endif // TTY_T_H