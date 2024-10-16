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
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, 
            GATE_ATTR_TYPE_INTR | GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_ATTR_D);
    }

    // 安装0号异常，divider
    irq_install(IRQ0_DE, (irq_handler_t)exception_handler_divider);

    lidt((uint32_t)idt_table, sizeof(idt_table));
}

// 安装irq
int irq_install(int irq_num, irq_handler_t handler) {
    if(irq_num >= IDT_TABLE_SIZE) {
        return -1;
    }

    // 设置门描述符，将irq_num这个异常号与handler绑定
    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler, 
        GATE_ATTR_TYPE_INTR | GATE_ATTR_P | GATE_ATTR_DPL0 | GATE_ATTR_D);
}

void do_default_handler(exception_frame_t* frame, const char* msg) {
    for(;;);
}

// unknown, -1
void do_handler_unknown(exception_frame_t* frame) {
    do_default_handler(frame, "unknown exception\n");
}

// divider, 0
void do_handler_divider(exception_frame_t* frame) {
    do_default_handler(frame, "divider exception\n");
}