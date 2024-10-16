/**
 * 
 * 中断相关基本类型
 * 
 */
#ifndef IRQ_T_H
#define IRQ_T_H

#include "common/types.h"

// 异常号，未标注的中断类型皆为 Fault
#define IRQ0_DE     0       // Divide Error
#define IRQ1_DB     1       // Debug Exception, Fault/Trap
#define IRQ2_NMI    2       // NMI interrupt, Interrupt
#define IRQ3_BP     3       // Breakpoint, Trap
#define IRQ4_OF     4       // Overflow, Trap
#define IRQ5_BR     5       // BOUND Range Exceeded
#define IRQ6_UD     6       // Invalid Opcode (Undefined Opcode)
#define IRQ7_NM     7       // Device Not Available (No Math Coprocessor)
#define IRQ8_DF     8       // Double Fault, Abort
// 9, Coprocessor Segment Overrun (reversed)
#define IRQ10_TS    10      // Invalid TSS
#define IRQ11_NP    11      // Segment Not Present
#define IRQ12_SS    12      // Stack-Segment Fault
#define IRQ13_GP    13      // General Protection
#define IRQ14_PF    14      // Page Fault
#define IRQ16_MF    16      // x87 FPU Floating-Point Error (Math Fault)
// 15, (Intel reversed. Do not used.)
#define IRQ17_AC    17      // Alignment Check
#define IRQ18_MC    18      // Machine Check, Abort
#define IRQ19_XM    19      // SMD Floating-Point Exception
#define IRQ20_VE    20      // Virtualization Exception
#define IRQ21_CP    21      // Control Protection Exception

// 22 ~ 31, intel reversed. Do not used.
// 32 ~ 255, User Defined (Non-reserved) interrupts, Interrupt


// 
#define IRQ0_TIMER              0x20
#define IRQ1_KEYBOARD		    0x21        // 按键中断
#define IRQ14_HARDDISK_PRIMARY  0x2E		// 主总线上的ATA磁盘中断

#define ERR_PAGE_P      (1 << 0)
#define ERR_PAGE_WR     (1 << 1)
#define ERR_PAGE_US     (1 << 1)

#define ERR_EXT         (1 << 0)
#define ERR_IDT         (1 << 1)



typedef struct _exception_frame_t{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t num, errno, eip, cs, eflags;
}exception_frame_t;


// 回调函数类型
typedef void (*irq_handler_t)();

#endif // IRQ_T_H