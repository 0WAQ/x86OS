/**
 * 
 * 与x86的体系结构相关的类型
 * 
 */
#ifndef CPU_T_H
#define CPU_T_H

#include "common/types.h"

#pragma pack(1)


// 段描述符表的属性
#define DESC_ATTR_G         (1 << 15)   // 粒度位
#define DESC_ATTR_D         (1 << 14)
#define DESC_ATTR_B         (1 << 14)
#define DESC_ATTR_L         (1 << 13)
#define DESC_ATTR_AVL       (1 << 12)
// 11 ~ 8这4位是limit的16 ~ 20位
#define DESC_ATTR_P         (1 << 7)    // 存在位

// DPL
#define DESC_ATTR_DPL0      (0 << 5)    // 特权级
#define DESC_ATTR_DPL1      (1 << 5)
#define DESC_ATTR_DPL2      (2 << 5)
#define DESC_ATTR_DPL3      (3 << 5)

// S
#define DESC_ATTR_S_SYS     (0 << 4)    // S位，用来表示是系统段还是用户段
#define DESC_ATTR_S_USR     (1 << 4)

// 以下为TYPE段，从左到右表示
#define DESC_ATTR_TYPE_DATA (0 << 3)
#define DESC_ATTR_TYPE_CODE (1 << 3)

// 表示数据段时一定可读，Read-Only，Expand-down(向下增长)，Write，Accessed(是否访问过)
#define DESC_ATTR_TYPE_R    (0)
#define DESC_ATTR_TYPE_RA   (1)
#define DESC_ATTR_TYPE_RW   (2)
#define DESC_ATTR_TYPE_RWA  (3)
#define DESC_ATTR_TYPE_RE   (4)
#define DESC_ATTR_TYPE_REA  (5)
#define DESC_ATTR_TYPE_RWE  (6)
#define DESC_ATTR_TYPE_RWEA (7)

// 表示代码段时一定可执行，Execute-Only，Conforming(一致性)，Read，Accessed
#define DESC_ATTR_TYPE_E    (0)
#define DESC_ATTR_TYPE_EA   (1)
#define DESC_ATTR_TYPE_ER   (2)
#define DESC_ATTR_TYPE_ERA  (3)
#define DESC_ATTR_TYPE_EC   (4)
#define DESC_ATTR_TYPE_ECA  (5)
#define DESC_ATTR_TYPE_ERC  (6)
#define DESC_ATTR_TYPE_ERCA (7)

typedef struct _segment_desc_t {
    uint16_t limit15_0;     // 0 ~ 15
    uint16_t base15_0;      // 16 ~ 31

    uint8_t  base23_16;     // 0 ~ 7
    uint16_t attr;          // 8 ~ 23
    uint8_t  base24_31;     // 24 ~ 31

}segment_desc_t;

#pragma pack()

#endif // CPU_T_H