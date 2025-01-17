/***
 * 
 * init
 * 
 */

#include "init.h"
#include "lib_syscall.h"

int main(int argc, char** argv) {
    int a = 3 / 0;
    *(char*)0 = 0x1234;
    return 0;
}