/***
 * 
 * 命令行解释器
 * 
 */

#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char** argv) {

    printf("Hello from shell\n");

    fork();
    yield();

    for(;;) {
        print("shell pid=%d", getpid());
        msleep(1000);
    }
}