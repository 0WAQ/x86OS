#include "sys/syscall.h"

int sys_fstat(int fd, struct stat* st) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }

    fs_t* fs = p->fs;
    kernel_memset(st, 0, sizeof(struct stat));

    int ret = -1;
    if(fs->op->stat != NULL) {
        fs_lock(fs);
        ret = fs->op->stat(p, st);
        fs_unlock(fs);
    }
    return ret;
}