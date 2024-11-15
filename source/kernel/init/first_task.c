/**
 * 
 * first_task相关c文件
 * 
 */

#include "applib/lib_syscall.h"

int first_task_main() {
    int pid = fork();
    if(pid == 0) {
        char* argv[] = {"arg0", "arg1", "arg2", "arg3"};
        execve("/shell.elf", argv, (char**)0);
    }

    for(;;) {
        print("pid = %d", getpid());
        msleep(1000);
    }
    return 0;
}
