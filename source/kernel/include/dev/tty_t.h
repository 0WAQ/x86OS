/**
 * 
 * tty 基本类型
 * 
 */
#ifndef TTY_T_H
#define TTY_T_H

#include "dev/dev_t.h"

#define TTY_NR              (8)

#define TTY_OBUF_SIZE       (512)
#define TTY_IBUF_SIZE       (512)

/**
 * @brief tty缓冲区
 */
typedef struct _tty_fifo_t {
    char* buf;
    int size;
    int read, write;
    int cnt;
}tty_fifo_t;

/**
 * @brief tty结构
 */
typedef struct _tty_t {
    char obuf[TTY_OBUF_SIZE];
    char ibuf[TTY_IBUF_SIZE];
    tty_fifo_t ofifo;
    tty_fifo_t ififo;
    int console_index;
}tty_t;

#endif // TTY_T_H