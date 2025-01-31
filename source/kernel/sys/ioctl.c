#include "sys/syscall.h"

int sys_ioctl(int fd, int cmd, int arg0, int arg1) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
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