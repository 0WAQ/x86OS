/**
 * 
 * 设备描述c文件
 * 
 */
#include "dev/dev.h"
#include "cpu/irq.h"
#include "tools/klib.h"

// 带注册的设备
extern dev_desc_t dev_tty_desc;

/**
 * @brief 设备描述表
 */
static dev_desc_t* dev_desc_tbl[] = {
    &dev_tty_desc, 
};

/**
 * @brief 设备表
 */
static device_t dev_tbl[DEV_TABLE_SIZE];

int dev_open(int major, int minor, void* data) {
    irq_state_t state = irq_enter_protection();
    
    device_t* fdev = NULL;
    for(int i = 0; i < DEV_TABLE_SIZE; i++) {
        device_t* dev = dev_tbl + i;
        
        // 找到空闲的表项
        if(dev->open_cnt == 0) {
            fdev = dev;
        }
        // 找到相同的设备, 直接返回其dev_id
        else if((dev->desc->major == major) && (dev->minor = minor)) {
            dev->open_cnt++;
            irq_leave_protectoin(state);
            return i;
        }
    }

    // 在设备描述表中寻找相同的major
    dev_desc_t* desc = NULL;
    for(int i = 0; i < sizeof(dev_desc_tbl) / sizeof(dev_desc_tbl[0]); i++) {
        dev_desc_t* d = dev_desc_tbl[i];
        if(d->major == major) {
            desc = d;
            break;
        }
    }

    // 如果desc于fdev都能找到, 初始化并返回dev_id(fdev- dev_tbl)
    if(desc && fdev) {
        fdev->minor = minor;
        fdev->data = data;
        fdev->desc = desc;
        
        int ret = desc->open(fdev);
        if(ret == 0) {
            fdev->open_cnt = 1;
            irq_leave_protectoin(state);
            return fdev - dev_tbl;
        }
    }

    irq_leave_protectoin(state);

    return -1;
}

int dev_read(int dev_id, int addr, char* buf, int size) {
    if(is_devid_bad(dev_id)) {
        return -1;
    }
    device_t* dev = dev_tbl + dev_id;
    return dev->desc->read(dev, addr, buf, size);
}

int dev_write(int dev_id, int addr, char* buf, int size) {
    if(is_devid_bad(dev_id)) {
        return -1;
    }
    device_t* dev = dev_tbl + dev_id;
    return dev->desc->write(dev, addr, buf, size);
}

int dev_control(int dev_id, int cmd, int arg0, int arg1) {
    if(is_devid_bad(dev_id)) {
        return -1;
    }
    device_t* dev = dev_tbl + dev_id;
    return dev->desc->control(dev, cmd, arg0, arg1);
}

void dev_close(int dev_id) {
    if(is_devid_bad(dev_id)) {
        return;
    }

    irq_state_t state = irq_enter_protection();

    device_t* dev = dev_tbl + dev_id;
    dev->desc = NULL;
    if(--dev->open_cnt == 0) {
        dev->desc->close(dev);
        kernel_memset((void*)dev, 0, sizeof(device_t));
    }
    
    irq_leave_protectoin(state);
}

static int is_devid_bad(int dev_id) {
    if((dev_id < 0) && (dev_id >= DEV_TABLE_SIZE)) {
        return 1;
    }

    if(dev_tbl[dev_id].desc == NULL) {
        return 1;
    }

    return 0;
}