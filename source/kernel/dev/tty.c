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
#include "cpu/irq.h"
#include "tools/log.h"

/**
 * @brief tty的设备层回调函数
 */
dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close
};

static tty_t tty_devs[TTY_NR];
static int curr_tty_idx = 0;

static void tty_fifo_init(tty_fifo_t* fifo, char* buf, int len) {
    fifo->buf = buf;
    fifo->size = len;
    fifo->cnt = 0;
    fifo->read = fifo->write = 0;
}

static int tty_open(device_t* dev) {

    int idx = dev->minor;
    if(idx < 0 || idx > TTY_NR) {
        log_print("open tty failed. incorrect tty idx = %d.", idx);
        return -1;
    }

    tty_t* tty =  tty_devs + idx;
    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    tty->console_index = idx;

        // 默认开启
    tty->oflags = TTY_OCRLF;
    tty->iflags = TTY_ICRLF | TTY_IECHO;

        // 资源是输出缓冲区中可写的字节数, 初始大小为缓冲区的大小
    sem_init(&tty->osem, TTY_OBUF_SIZE);
    
        // 资源是输入缓冲区需要读的字节数, 初始大小为0
    sem_init(&tty->isem, 0);

    kbd_init();
    console_init(idx);

    return 0;
}

static int tty_read(device_t* dev, int addr , char* buf , int size) {
    if(size < 0) {
        return -1;
    }

    tty_t* tty = get_tty(dev);
    char* p = buf;

    int len = 0;
    while(len < size) {

            // 这里是消费者, 会消耗资源(输入缓冲区中的有效载荷), 没有资源会睡眠, 默认为0
        sem_wait(&tty->isem);
    
        char ch;
        tty_fifo_get(&tty->ififo, &ch);
        

        switch (ch)
        {
        case '\n':
            // 根据配置, 决定是否将 \n 转换为 \r\n
            if(tty->iflags & TTY_ICRLF) {
                // 还要确保缓冲区大小足够
                if(len < size - 1) {
                    *p++ = '\r';
                    len++;
                }
            }
            *p++ = ch;
            len++;
            break;
        
        case 0x7F:
            if(len == 0) {
                continue;
            }
            p--;
            len--;

            break;

        default:
            *p++ = ch;
            len++;
            break;
        }

        // 若开启回显
        if(tty->iflags & TTY_IECHO) {
            tty_write(dev, 0, &ch, 1);
        }

        // 碰到回车或者换行, 提前终止, 是输入缓冲区回显
        if((ch == '\n') || (ch == '\r')) {
            break;
        }
    }

    return len;
}

static int tty_write(device_t* dev, int addr , char* buf , int size) {
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

static int tty_control(device_t* dev, int cmd, int arg0, int arg1) {
    tty_t* tty = get_tty(dev);
    switch (cmd)
    {
    case TTY_CMD_ECHO:
        if(arg0) {
            tty->iflags |= TTY_IECHO;
            set_cursor_visiable(tty->console_index, 1); // 光标可见
        }
        else {
            tty->iflags &= ~TTY_IECHO;
            set_cursor_visiable(tty->console_index, 0); // 光标不可见
        }
        break;

    case TTY_CMD_IN_COUNT:
        if(arg0) {
            *(int*)arg0 = sem_count(&tty->isem);
        }
        break;

    default:
        break;
    }
    
    return 0;
}

static void tty_close(device_t* dev) {

}

static tty_t* get_tty(device_t* dev) {
    int minor = dev->minor;
    if(minor < 0 || minor >= TTY_NR || !dev->open_cnt) {
        log_print("tty is not opened. tty = %d.", minor);
        return NULL;
    }
    return tty_devs + minor;
}

static int tty_fifo_put(tty_fifo_t* fifo, char ch) {

    irq_state_t state = irq_enter_protection();

    if(fifo->cnt >= fifo->size) {
        irq_leave_protection(state);
        return -1;
    }

    fifo->buf[fifo->write] = ch;
    if(++fifo->write >= fifo->size) {
        fifo->write = 0;
    }
    fifo->cnt++;

    irq_leave_protection(state);

    return 0;
}

int tty_fifo_get(tty_fifo_t* fifo, char* ch) {

    irq_state_t state = irq_enter_protection();

    if(fifo->cnt <= 0) {
        irq_state_t state = irq_enter_protection();
        return -1;
    }

    *ch = fifo->buf[fifo->read];
    if(++fifo->read >= fifo->size) {
        fifo->read = 0;
    }
    fifo->cnt--;

    irq_leave_protection(state);

    return 0;
}

void tty_in(char ch) {
    tty_t* tty = tty_devs + curr_tty_idx;

    // 输入缓冲区的有效字节数等于缓冲区大小, 那么满, 直接返回
    if(sem_count(&tty->isem) >= TTY_IBUF_SIZE) {
        return;
    }

    // 否则将ch放入到输入缓冲区
    tty_fifo_put(&tty->ififo, ch);
    sem_notify(&tty->isem);
}

void tty_switch(int idx) {
    if(idx != curr_tty_idx) {
        console_switch(idx);
        curr_tty_idx = idx;
    }
}