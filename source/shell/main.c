/***
 * 
 * 命令行解释器
 * 
 */

#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char** argv) {

#if 0

    void* ret = sbrk(0);
    ret = sbrk(100);
    ret = sbrk(200);
    ret = sbrk(4096 * 2 + 200);
    ret = sbrk(4096 * 5 + 1234);

    printf("Hello from shell\n");

    printf("\0337Hello,word!\0338123\n");  // ESC 7,8 输出123lo,word!
    printf("\033[31;42mHello,word!\033[39;49m123\n");  // ESC [pn m, Hello,world红色，>其余绿色
    printf("123\033[2DHello,word!\n");  // 光标左移2，1Hello,word!
    printf("123\033[2CHello,word!\n");  // 光标右移2，123  Hello,word!

    printf("\033[31m\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[10;10H test!\n");  // 定位到10, 10，test!
    printf("\033[20;20H test!\n");  // 定位到20, 20，test!
    printf("\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[2J");

#endif

    int fd = open(argv[0], 0);
    dup(fd);
    dup(fd);

    printf("Hello from shell\n");

    char buf[256] = {0};
    for(;;) {
        printf("shell pid = %d\n", getpid());
        gets(buf);
        puts(buf);
    }
}