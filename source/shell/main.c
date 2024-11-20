/***
 * 
 * 命令行解释器
 * 
 */

#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char** argv) {

    void* ret = sbrk(0);
    ret = sbrk(100);
    ret = sbrk(200);
    ret = sbrk(4096 * 2 + 200);
    ret = sbrk(4096 * 5 + 1234);

    printf("Hello from shell\n");

    if(fork() > 0) {
        for(;;) {
            printf("child shell pid = %d\n", getpid());
            msleep(1000);
        }
    }

    for(;;) {
        printf("shell pid = %d\n", getpid());
        msleep(1000);
    }
}