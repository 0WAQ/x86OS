/***
 * 
 * 命令行解释器
 * 
 */

#include "lib_syscall.h"

int main(int argc, char** argv) {

    for(int i = 0; i < argc; ++i) {
        print("arg: %s", (int)argv[i]);
    }

    for(;;) {
        msleep(1000);
    }
}