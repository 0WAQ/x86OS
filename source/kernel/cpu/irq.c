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
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)default_exception_handler, 
            GATE_ATTR_TYPE_INTR | GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_ATTR_D);
    }

    lidt((uint32_t)idt_table, sizeof(idt_table));
}

void default_exception_handler_aux(exception_frame_t* frame) {
    do_default_handler(frame, "unknown exception\n");
}

void do_default_handler(exception_frame_t* frame, const char* msg) {
    for(;;);
}