/**
 * 
 * 
 * 
 */

int main(int, char**);

#include <stdint.h>

extern uint8_t __bss_start__[], __bss_end__[];

void cstart(int argc, char** argv) {

    // 清空bss区
    uint8_t* start = __bss_start__;
    while(start < __bss_end__) {
        *start++ = 0;
    }

    main(argc, argv);
}
