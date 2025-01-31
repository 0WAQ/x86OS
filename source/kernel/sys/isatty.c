#include "sys/syscall.h"

int sys_isatty(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }
    return p->type == FILE_TTY;
}