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

    // 安装异常处理接口
    irq_install(IRQ0_DE,  exception_handler_divider);
    irq_install(IRQ1_DB,  exception_handler_debug);
	irq_install(IRQ2_NMI, exception_handler_NMI);
	irq_install(IRQ3_BP,  exception_handler_breakpoint);
	irq_install(IRQ4_OF,  exception_handler_overflow);
	irq_install(IRQ5_BR,  exception_handler_bound_range);
	irq_install(IRQ6_UD,  exception_handler_invalid_opcode);
	irq_install(IRQ7_NM,  exception_handler_device_unavailable);
	irq_install(IRQ8_DF,  exception_handler_double_fault);
	irq_install(IRQ10_TS, exception_handler_invalid_tss);
	irq_install(IRQ11_NP, exception_handler_segment_not_present);
	irq_install(IRQ12_SS, exception_handler_stack_segment_fault);
	irq_install(IRQ13_GP, exception_handler_general_protection);
	irq_install(IRQ14_PF, exception_handler_page_fault);
	irq_install(IRQ16_MF, exception_handler_fpu_error);
	irq_install(IRQ17_AC, exception_handler_alignment_check);
	irq_install(IRQ18_MC, exception_handler_machine_check);
	irq_install(IRQ19_XM, exception_handler_smd_exception);
	irq_install(IRQ20_VE, exception_handler_virtual_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

	// 初始化中断控制器
	pic_init();
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

void pic_init(void) {
    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);

    // 对应的中断号起始序号0x20
    outb(PIC0_ICW2, IRQ_PIC_START);

    // 主片IRQ2有从片
    outb(PIC0_ICW3, 1 << 2);

    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC0_ICW4, PIC_ICW4_8086);

    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC1_ICW1, PIC_ICW1_ICW4 | PIC_ICW1_ALWAYS_1);

    // 起始中断序号，要加上8
    outb(PIC1_ICW2, IRQ_PIC_START + 8);

    // 没有从片，连接到主片的IRQ2上
    outb(PIC1_ICW3, 2);

    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC1_ICW4, PIC_ICW4_8086);

    // 禁止所有中断, 允许从PIC1传来的中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
}

void irq_disable_global() {
	cli();
}

void irq_enalbe_global() {
	sti();
}

void irq_enable(int irq_num) {
	if(irq_num < IRQ_PIC_START) {
		return;
	}
	
	irq_num -= IRQ_PIC_START;
	if(irq_num < 8) {
		uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
		outb(PIC0_IMR, mask);
	}
	else {
		uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
		outb(PIC1_IMR, mask);
	}
}

void irq_disable(int irq_num) {
	if(irq_num < IRQ_PIC_START) {
		return;
	}
	
	irq_num -= IRQ_PIC_START;
	if(irq_num < 8) {
		uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
		outb(PIC0_IMR, mask);
	}
	else {
		uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
		outb(PIC1_IMR, mask);
	}
}

void do_default_handler(exception_frame_t* frame, const char* msg) {
    for(;;) { hlt(); }
}

/////////////////////////////////////////////////////////////////////////////////

// unknown, -1
void do_handler_unknown(exception_frame_t* frame) {
    do_default_handler(frame, "Unknown Exception.\n");
}

// divider, 0
void do_handler_divider(exception_frame_t* frame) {
    do_default_handler(frame, "Divider Exception.\n");
}

void do_handler_debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception.\n");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.\n");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.\n");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.\n");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.\n");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.\n");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.\n");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.\n");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.\n");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.\n");
}

void do_handler_general_protection(exception_frame_t * frame) {
	do_default_handler(frame, "General Protection.\n");
}

void do_handler_page_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Page Fault.\n");
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.\n");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.\n");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.\n");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.\n");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.\n");
}