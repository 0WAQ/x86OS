/**
 * 
 * 中断处理
 * 
 */
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "core/task.h"
#include "core/syscall.h"
#include "os_cfg.h"
#include "common/cpu_instr.h"
#include "tools/log.h"

// IDT
static gate_desc_t idt_table[IDT_TABLE_SIZE];

void irq_init() {
    for(int i = 0; i < IDT_TABLE_SIZE; i++) {
        set_gate_desc(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, 
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

	set_gate_desc(idt_table + 0x80, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_syscall, 
        GATE_ATTR_TYPE_INTR | GATE_ATTR_P | GATE_ATTR_DPL3);

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
    set_gate_desc(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler, 
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
		irq_num -= 8;
		uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
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
		irq_num -= 8;
		uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
		outb(PIC1_IMR, mask);
	}
}

void pic_send_eoi(int irq_num) {
	irq_num -= IRQ_PIC_START;

    // 从片也可能需要发送EOI
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }

    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

void do_default_handler(exception_frame_t* frame, const char* msg) {
	log_print("------------------------------");
	log_print("IRQ/Exception happened %s", msg);
	dump_core_regs(frame);

	// 判断cs的低2位(特权级别), 若是用户程序触发, 那么退出
	if(frame->cs & 0x3) {
		sys_exit(frame->errno);
	}
	else {
    	while(1) { 
			hlt();
		}
	}
}

void dump_core_regs(exception_frame_t* frame) {
	log_print("IRQ: %d, Error Code: %d", frame->num, frame->errno);
	
	uint32_t ss, esp;
	if(frame->cs & 0x3) {
		ss = frame->ss3;
		esp = frame->esp3;
	}
	else {
		ss = frame->ds;
		esp = frame->esp;
	}
	
	log_print("CS: %d\r\n"
			  "DS: %d\r\n"
			  "ES: %d\r\n"
			  "SS: %d\r\n"
			  "FS: %d\r\n"
			  "GS: %d", frame->cs, frame->ds, frame->es, ss, frame->fs, frame->gs);
	
	log_print("EAX: 0x%x\r\n"
              "EBX: 0x%x\r\n"
              "ECX: 0x%x\r\n"
              "EDX: 0x%x\r\n"
              "EDI: 0x%x\r\n"
              "ESI: 0x%x\r\n"
              "EBP: 0x%x\r\n"
              "ESP: 0x%x", frame->eax, frame->ebx, frame->ecx, frame->edx,
               				 frame->edi, frame->esi, frame->ebp, esp);

    log_print("EIP: 0x%x\r\n"
			  "EFLAGS: 0x%x\r\n", frame->eip, frame->eflags);
}

/////////////////////////////////////////////////////////////////////////////////

// unknown, -1
void do_handler_unknown(exception_frame_t* frame) {
    do_default_handler(frame, "Unknown Exception.");
}

// divider, 0
void do_handler_divider(exception_frame_t* frame) {
    do_default_handler(frame, "Divider Exception.");
}

void do_handler_debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception.");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.");
}

void do_handler_general_protection(exception_frame_t * frame) {

    log_print("--------------------------------");
    log_print("IRQ/Exception happend: General Protection.");

    if (frame->errno & ERR_EXT)
        log_print("the exception occurred during delivery of an \
                    event external to the program, such as an interrupt or an earlier exception.");
    else
    	log_print("the exception occurred during delivery of a \
					software interrupt (INT n, INT3, or INTO).");
    
    if (frame->errno & ERR_IDT)
        log_print("the index portion of the error code refers to a gate descriptor in the IDT");
    else
     	log_print("the index refers to a descriptor in the GDT");
    
    log_print("segment index: %d", frame->errno & 0xFFF8);

    dump_core_regs(frame);

	// 判断cs的低2位(特权级别), 若是用户程序触发, 那么退出
	if(frame->cs & 0x3) {
		sys_exit(frame->errno);
	}
	else {
    	while(1) { 
			hlt();
		}
	}
}

void do_handler_page_fault(exception_frame_t * frame) {

    log_print("--------------------------------");
    log_print("IRQ/Exception happend: Page fault.");

    if (frame->errno & ERR_PAGE_P)
    	log_print("  page-level protection violation: 0x%x.", read_cr2());
	else
        log_print("  Page doesn't present: 0x%x", read_cr2());
    
    if (frame->errno & ERR_PAGE_WR)
        log_print("  The access causing the fault was a write: 0x%x", read_cr2());
	else
        log_print("  The access causing the fault was a read: 0x%x", read_cr2());
    
    if (frame->errno & ERR_PAGE_US)
        log_print("  A user-mode access caused the fault: 0x%x", read_cr2());
	else
        log_print("  A supervisor-mode access caused the fault: 0x%x", read_cr2());

    dump_core_regs(frame);

	// 判断cs的低2位(特权级别), 若是用户程序触发, 那么退出
	if(frame->cs & 0x3) {
		sys_exit(frame->errno);
	}
	else {
    	while(1) { 
			hlt();
		}
	}
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.");
}


irq_state_t irq_enter_protection() {
	// 通过读eflags寄存器, 判断在进入临界区前, 中断开关的状态
	irq_state_t state = read_eflags();
	irq_disable_global();
	return state;
}

void irq_leave_protectoin(irq_state_t state) {
	// 复原进入临界区之前中断的状态
	write_eflags(state);
}