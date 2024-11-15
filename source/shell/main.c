/***
 * 
 * 命令行解释器
 * 
 */

#include "lib_syscall.h"

int main(int argc, char** argv) {

    fork();
    yield();

    for(;;) {
        print("shell pid=%d", getpid());
        msleep(1000);
    }
}