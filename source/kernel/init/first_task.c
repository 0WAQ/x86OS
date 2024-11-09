/**
 * 
 * first_task相关c文件
 * 
 */

#include "applib/lib_syscall.h"

int first_task_main() {
    int count = 3;
    
    int pid = fork();
    print("first_task id: %d", pid);
    if(pid < 0) {
        print("create child proc failed.", 0);
    }
    else if(pid == 0) {
        print("child: %d", count);
    }
    else {
        print("child task id: %d", pid);
        print("parent: %d", count);
    }

    int ppid = getpid();
    for(;;) {
        print("pid = %d", ppid);
        msleep(10);
    }

    return 0;
}
