/**
 * 
 * tty c文件
 * 
 */
#include "dev/tty.h"
#include "dev/dev.h"
#include "dev/console.h"
#include "dev/kbd.h"
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

static tty_t ttys_dev[TTY_NR];

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

    tty_t* tty =  ttys_dev + idx;
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    tty->console_index = idx;

    kbd_init();
    console_init(idx);

    return 0;
}

int tty_read(device_t* dev, int addr , char* buf , int size) {
    return size;
}

int tty_write(device_t* dev, int addr , char* buf , int size) {
    return size;
}

int tty_control(device_t* dev, int cmd, int arg0, int arg1) {
    return 0;
}

void tty_close(device_t* dev) {

}
