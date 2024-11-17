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
    printf("OS Version: %s\n", "1.0.0");

    fork();
    yield();

    for(;;) {
        printf("shell pid=%d\n", getpid());
        msleep(1000);
    }
}