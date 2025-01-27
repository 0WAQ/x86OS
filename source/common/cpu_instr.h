/**
 * 
 * 汇编指令的封装
 * 
 */
#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "types.h"

/**
 * @brief 停机指令
 */
static inline void hlt() {
    __asm__ __volatile__("hlt");
}

/**
 * @brief 关中断
 */
static inline void cli() {
    __asm__ __volatile__("cli");
}

/**
 * @brief 开中断
 */
static inline void sti() {
    __asm__ __volatile__("sti");
}

/**
 * @brief 读端口，inb al, dx
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__(
        "inb %[p], %[v]"
        : [v]"=a" (data)    // 输出端口
        : [p]"d" (port)     // 输入端口
    );
    return data;
}

/**
 * @brief 写端口，outb al, dx
 */
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__(
        "outb %[v], %[p]"
        :
        : [p]"d" (port), [v]"a" (data)
    );
}

// TODO:
#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})

/**
 * @brief 读端口，inw ax, dx
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t data;
    __asm__ __volatile__(
        "in %[p], %[v]"
        : [v]"=a" (data)    // 输出端口
        : [p]"d" (port)     // 输入端口
    );
    return data;
}

/**
 * @brief 写端口，outw ax, dx
 */
static inline void outw(uint16_t port, uint16_t data) {
    __asm__ __volatile__(
        "out %[v], %[p]"
        :
        : [p]"d" (port), [v]"a" (data)
    );
}

/**
 * @brief 加载全局段描述符表
 */
static inline void lgdt(uint32_t start, uint32_t size) {
    
    // 顺序不能变
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }gdt;
    
    gdt.start31_16 = start >> 16;
    gdt.start15_0 = start & 0xFFFF;
    gdt.limit = size - 1;

    __asm__ __volatile__(
        "lgdt %[addr]"
        :
        : [addr]"m" (gdt)
    );
}

/**
 * @brief 加载中断描述符表
 */
static inline void lidt(uint32_t start, uint32_t size) {
    
    // 顺序不能变
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }idt;
    
    idt.start31_16 = start >> 16;
    idt.start15_0 = start & 0xFFFF;
    idt.limit = size - 1;

    __asm__ __volatile__(
        "lidt %[addr]"
        :
        : [addr]"m" (idt)
    );
}

/**
 * @brief 加载tss到任务寄存器
 */
static inline void ltr(uint16_t tss_sel) {
    __asm__ __volatile__(
        "ltr %%ax"
        :
        : "a"(tss_sel)
    );
}

/**
 * @brief 读取CRX
 */
static inline uint32_t read_cr0() {
    uint32_t cr0;
    __asm__ __volatile__(
        "mov %%cr0, %[v]"
        : [v]"=r" (cr0)
        :
    );
    return cr0;
}

static inline uint32_t read_cr2() {
    uint32_t cr2;
    __asm__ __volatile__(
        "mov %%cr2, %[v]"
        : [v]"=r" (cr2)
        :
    );
    return cr2;
}

static inline uint32_t read_cr3() {
    uint32_t cr3;
    __asm__ __volatile__(
        "mov %%cr3, %[v]"
        : [v]"=r" (cr3)
        :
    );
    return cr3;
}

static inline uint32_t read_cr4() {
    uint32_t cr4;
    __asm__ __volatile__(
        "mov %%cr4, %[v]"
        : [v]"=r" (cr4)
        :
    );
    return cr4;
}

/**
 * @brief 写CRX
 */
static inline void write_cr0(uint32_t cr0) {
        __asm__ __volatile__(
        "mov %[v], %%cr0"
        :
        : [v]"r" (cr0)
    );
}

static inline void write_cr2(uint32_t cr2) {
        __asm__ __volatile__(
        "mov %[v], %%cr2"
        :
        : [v]"r" (cr2)
    );
}

static inline void write_cr3(uint32_t cr3) {
        __asm__ __volatile__(
        "mov %[v], %%cr3"
        :
        : [v]"r" (cr3)
    );
}

static inline void write_cr4(uint32_t cr4) {
        __asm__ __volatile__(
        "mov %[v], %%cr4"
        :
        : [v]"r" (cr4)
    );
}

/**
 * @brief 远跳转指令
 */
static inline void far_jump(uint32_t selector, uint32_t offset) {
    
    // 小端字节序, 高字节放在低地址处, 所以offset在低地址, selector在高地址
    uint32_t addr[] = {offset, selector};
    __asm__ __volatile__(
        "ljmpl *(%[a])"
        :
        : [a]"r" (addr)
    );
}

/**
 * @brief 读eflags寄存器
 */
static inline uint32_t read_eflags() {
    uint32_t eflags;
    __asm__ __volatile__(
        "pushf\n\t"
        "pop %%eax"
        :"=a"(eflags)
        :
    );
    return eflags;
}

/**
 * @brief 写eflags寄存器
 */
static inline void write_eflags(uint32_t eflags) {
    __asm__ __volatile__(
        "push %%eax\n\t"
        "popf"
        :
        :"a"(eflags)
    );
}

#endif // CPU_INSTR_H