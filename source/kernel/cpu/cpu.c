/**
 * 
 * CPU设置
 * 
 */
#include "cpu/cpu.h"
#include "os_cfg.h"

// gdt
static segment_desc_t gdt_table[GDT_TABLE_SIZE];

void cpu_init() {
    
    // 初始化gdt
    gdt_init();

    // 
}

void gdt_init() {
    for(int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i << 3, 0, 0, 0);
    }
}

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t* desc = gdt_table + (selector >> 3); // 每个gdt的大小是8字节(64位)
    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
    desc->base24_31 = (base >> 24) & 0xFF;
}
