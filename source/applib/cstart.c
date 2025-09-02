#include <stdlib.h>

#include "common/types.h"

extern u8_t __bss_start__[], __bss_end__[];

int main(int, char**);

void cstart(int argc, char** argv) {

    // 清空bss区
    u8_t* start = __bss_start__;
    while(start < __bss_end__) {
        *start++ = 0;
    }

    int ret = main(argc, argv);
    exit(ret);
}
