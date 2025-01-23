/**
 * 
 * first_task相关c文件
 * 
 */

#include "applib/lib_syscall.h"
#include "dev/tty.h"

#include <stdio.h>

int first_task_main() {

    char tty[] = "/dev/tty?";
    for(int i = 0; i < 1; i++) {
        int pid = fork();
        if(pid < 0) {
            printf("create shell failed.\n");
            break;
        }
        else if(pid == 0) {
            tty[sizeof(tty) - 2] = i + '0';
            char* argv[] = {tty, NULL};
            execve("shell.elf", argv, NULL);
            while(1) {
                printf("execve failed.\n");
                msleep(1000);
            }
        }
    }

    for(;;) {
        int status;
        wait(&status);
    }
    return 0;
}
