#include "sys/syscall.h"

/**
 * @brief 系统调用: 对输入输出设备进行控制 TODO: 将其改为可变参传递(前提: 将系统调用改为可变参传递)
 */
int sys_ioctl(int fd, int cmd, int arg0, int arg1) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = task_get_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->ioctl != NULL) {
        fs_lock(fs);
        ret = fs->op->ioctl(p, cmd, arg0, arg1);
        fs_unlock(fs);
    }
    return ret;
}