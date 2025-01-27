/**
 * 
 * 与x86的体系结构相关的类型
 * 
 */
#ifndef CPU_T_H
#define CPU_T_H

#include "common/types.h"

////// 段描述符表的属性
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

// RPL
#define DESC_ATTR_RPL0      (0 << 0)
#define DESC_ATTR_RPL1      (1 << 0)
#define DESC_ATTR_RPL2      (2 << 0)
#define DESC_ATTR_RPL3      (3 << 0)

// S
#define DESC_ATTR_S_SYS     (0 << 4)    // S位，用来表示是系统段还是用户段
#define DESC_ATTR_S_USR     (1 << 4)

// 以下为TYPE段，从左到右表示
#define DESC_ATTR_TYPE_DATA (0 << 3)
#define DESC_ATTR_TYPE_CODE (1 << 3)
#define DESC_ATTR_TYPE_TSS  (9 << 0)

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


////// 中断表中的属性
#define GATE_ATTR_P         (1 << 15)

// DPL属性
#define GATE_ATTR_DPL0      (0 << 13)
#define GATE_ATTR_DPL1      (1 << 13)
#define GATE_ATTR_DPL2      (2 << 13)
#define GATE_ATTR_DPL3      (3 << 13)

// D位，任务门始终为0
#define GATE_ATTR_D         (1 << 11)

// 调用门类型
#define GATE_ATTR_TYPE_TASK     (5 << 8)    // 任务门
#define GATE_ATTR_TYPE_INTR     (0xE << 8)    // 中断门
#define GATE_ATTR_TYPE_TRAP     (7 << 8)    // 陷阱门
#define GATE_ATTR_TYPE_SYSCALL  (0xC << 8)    // 调用门

// 初始化tss时eflags的状态
#define EFLAGS_DEFAULT      (1 << 1)
#define EFLAGS_IF           (1 << 9)

/**
 * GDT表项，段描述符
 */
typedef struct _segment_desc_t {
    u16_t limit15_0;     // 0 ~ 15
    u16_t base15_0;      // 16 ~ 31

    u8_t  base23_16;     // 0 ~ 7
    u16_t attr;          // 8 ~ 23
    u8_t  base24_31;     // 24 ~ 31

}__attribute__((packed)) segment_desc_t;

/**
 * IDT表项(三者中的中断门)，调用门描述符
 */
typedef struct _gate_desc_t {
    u16_t offset15_0;    // 0 ~ 15
    u16_t selector;      // 16 ~ 31
    u16_t attr;          // 0 ~ 15
    u16_t offset31_16;   // 16 ~ 31
}__attribute__((packed)) gate_desc_t;

/**
 * 任务状态段
 */
typedef struct _tss_t {
    u32_t pre_link;
    u32_t esp0, ss0, esp1, ss1, esp2, ss2;
    u32_t cr3;
    u32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u32_t es, cs, ss, ds, fs, gs;
    u32_t ldt;
    u32_t iomap;     // 控制IO访问相关信息
    // u32_t ssp;
}__attribute__((packed)) tss_t;

#endif // CPU_T_H