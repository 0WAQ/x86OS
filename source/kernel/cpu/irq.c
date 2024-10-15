/**
 * 
 * 中断处理
 * 
 */
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"
#include "common/cpu_instr.h"

// IDT
static gate_desc_t idt_table[IDT_TABLE_SIZE];

void irq_init() {
    for(int i = 0; i < IDT_TABLE_SIZE; i++) {
        gate_desc_set(idt_table + i, 0, 0, 0);
    }

    lidt((uint32_t)idt_table, sizeof(idt_table));
}