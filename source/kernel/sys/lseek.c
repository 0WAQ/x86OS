#include "sys/syscall.h"

int sys_lseek(int fd, int offset, int dir) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->seek != NULL) {
        fs_lock(fs);
        ret = fs->op->seek(p, offset, dir);
        fs_unlock(fs);
    }
    return ret;
}