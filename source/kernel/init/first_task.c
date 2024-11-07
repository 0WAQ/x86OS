/**
 * 
 * first_task相关c文件
 * 
 */

#include "applib/lib_syscall.h"

void first_task_main() {
    int pid = getpid();
    for(;;) {
        msleep(10);
    }
}
