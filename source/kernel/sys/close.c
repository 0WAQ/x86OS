#include "sys/syscall.h"

int sys_close(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }

    // 断言文件已经打开
    ASSERT(p->ref > 0);

    // 减少ref
    if(--p->ref == 0) { // 若为0, 那么关闭文件
        fs_t* fs = p->fs;
        if(fs->op->close != NULL) {
            fs_lock(fs);
            fs->op->close(p);
            fs_unlock(fs);
        }
        file_dec_ref(p);
    }
    // 释放fd
    task_remove_fd(fd);
    return 0;
}