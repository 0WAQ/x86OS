/**
 * 
 * first_task相关c文件
 * 
 */

#include "applib/lib_syscall.h"
#include "dev/tty.h"

int first_task_main() {

    char tty[5] = "tty:?";
    for(int i = 0; i < TTY_NR; i++) {
        int pid = fork();
        if(pid < 0) {
            print("create shell failed.", 0);
            break;
        }
        else if(pid == 0) {
            tty[4] = i + '0';
            char* argv[] = {tty, NULL};
            execve("/shell.elf", argv, NULL);
            while(1) {
                print("execve failed.", 0);
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
