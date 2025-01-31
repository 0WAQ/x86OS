#include "sys/syscall.h"

int sys_dup(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    // 新增new指向fd对应的文件
    int new = task_alloc_fd(p);
    if(new < 0) {
        log_print("no task file avaliable.");
        return -1;
    }
    
    // 增加文件的引用计数
    file_inc_ref(p);
    return new;
}