/**
 * 
 * 与x86的体系结构相关的类型
 * 
 */
#ifndef CPU_T_H
#define CPU_T_H

#include "common/types.h"

#pragma pack(1)

typedef struct _segment_desc_t {
    uint16_t limit15_0;     // 0 ~ 15
    uint16_t base15_0;      // 16 ~ 31

    uint8_t  base23_16;     // 0 ~ 7
    uint16_t attr;          // 8 ~ 23
    uint8_t  base24_31;     // 24 ~ 31

}segment_desc_t;

#pragma pack()

#endif // CPU_T_H