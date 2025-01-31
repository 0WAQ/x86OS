#include "sys/syscall.h"

int sys_write(int fd, char* buf, int len) {
    if(is_fd_bad(fd) || buf == NULL || len == 0) {
        return 0;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    // 若文件是只写的
    if(p->mode == O_RDONLY) {
        log_print("file is write only");
        return -1;
    }

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->write != NULL) {
        fs_lock(fs);
        ret = fs->op->write(buf, len, p);
        fs_unlock(fs);
    }
    return ret;
}