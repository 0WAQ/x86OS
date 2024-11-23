/**
 * 
 * tty c文件
 * 
 */
#include "dev/tty.h"
#include "dev/dev.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "ipc/sem.h"
#include "tools/log.h"

/**
 * @brief tty的描述结构
 */
dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};

static tty_t ttys_devs[TTY_NR];

void tty_fifo_init(tty_fifo_t* fifo, char* buf, int len) {
    fifo->buf = buf;
    fifo->size = len;
    fifo->cnt = 0;
    fifo->read = fifo->write = 0;
}

int tty_open(device_t* dev) {

    int idx = dev->minor;
    if(idx < 0 || idx > TTY_NR) {
        log_print("open tty failed. incorrect tty idx = %d.", idx);
        return -1;
    }

    tty_t* tty =  ttys_devs + idx;
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    tty->console_index = idx;

        // 默认开启
    tty->oflags = TTY_OCRLF;

        // 资源是输出缓冲区中可写的字节数, 初始大小为缓冲区的大小
    sem_init(&tty->osem, TTY_OBUF_SIZE);
    // sem_init(&tty->isem, TTY_IBUF_SIZE);

    kbd_init();
    console_init(idx);

    return 0;
}

int tty_read(device_t* dev, int addr , char* buf , int size) {
    return size;
}

int tty_write(device_t* dev, int addr , char* buf , int size) {
    if(size < 0) {
        return -1;
    }

    tty_t* tty = get_tty(dev);
    if(tty == NULL) {
        return -1;
    }

    // 将所有的数据写入缓冲区
    int len = 0;
    while(size) {
        char ch = *buf++;

        // 根据配置, 决定是否将\n转换成\r\n
        if(ch == '\n' && (tty->oflags & TTY_OCRLF)) {
            sem_wait(&tty->osem);
            int ret = tty_fifo_put(&tty->ofifo, '\r');
            if(ret < 0) {
                break;
            }
        }

            // 这里消费者, 会消耗资源(缓冲区中的空闲字节), 没有资源会睡眠 
        sem_wait(&tty->osem);

            // 消耗资源向缓冲区写入数据
        int ret = tty_fifo_put(&tty->ofifo, ch);
        if(ret < 0) {
            break;
        }
        ++len, --size;
        
        // 读取到数据后, 将其写入控制台
        console_write(tty);
    }

    return len;
}

int tty_control(device_t* dev, int cmd, int arg0, int arg1) {
    return 0;
}

void tty_close(device_t* dev) {

}

static tty_t* get_tty(device_t* dev) {
    int minor = dev->minor;
    if(minor < 0 || minor >= TTY_NR || !dev->open_cnt) {
        log_print("tty is not opened. tty = %d.", minor);
        return NULL;
    }
    return ttys_devs + minor;
}

int tty_fifo_put(tty_fifo_t* fifo, char ch) {
    if(fifo->cnt >= fifo->size) {
        return -1;
    }

    fifo->buf[fifo->write] = ch;
    if(++fifo->write >= fifo->size) {
        fifo->write = 0;
    }
    fifo->cnt++;
    return 0;
}

int tty_fifo_get(tty_fifo_t* fifo, char* ch) {
    if(fifo->cnt <= 0) {
        return -1;
    }

    *ch = fifo->buf[fifo->read];
    if(++fifo->read >= fifo->size) {
        fifo->read = 0;
    }
    fifo->cnt--;
    return 0;
}