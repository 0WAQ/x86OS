/***
 * 
 * 中断处理头文件
 * 
 */
#ifndef IRQ_H
#define IRQ_H

#include "irq_t.h"

/**
 * @brief 初始化中断描述符表
 */
void irq_init();

/**
 * @brief 将对应的异常与异常处理函数绑定
 */
int irq_install(int irq_num, irq_handler_t handler);

/**
 * @brief 初始化pic
 */
void pic_init();

/**
 * @brief 关中断
 */
void irq_disable_global();

/**
 * @brief 开中断
 */
void irq_enalbe_global();

/**
 * @brief 开启指定中断
 */
void irq_enable(int irq_num);

/**
 * @brief 关闭指定中断
 */
void irq_disable(int irq_num);

/**
 * @brief 异常处理程序的执行函数
 */
void do_default_handler(exception_frame_t* frame, const char* msg);


/**
 * @brief 异常处理程序，在init/start.S中实现，与异常号绑定
 */
void exception_handler_unknown();
void exception_handler_divider();
void exception_handler_debug (void);
void exception_handler_NMI (void);
void exception_handler_breakpoint (void);
void exception_handler_overflow (void);
void exception_handler_bound_range (void);
void exception_handler_invalid_opcode (void);
void exception_handler_device_unavailable (void);
void exception_handler_double_fault (void);
void exception_handler_invalid_tss (void);
void exception_handler_segment_not_present (void);
void exception_handler_stack_segment_fault (void);
void exception_handler_general_protection (void);
void exception_handler_page_fault (void);
void exception_handler_fpu_error (void);
void exception_handler_alignment_check (void);
void exception_handler_machine_check (void);
void exception_handler_smd_exception (void);
void exception_handler_virtual_exception (void);

/**
 * @brief 异常处理程序，C实现
 */
void do_handler_unknown(exception_frame_t* frame);
void do_handler_divider(exception_frame_t* frame);
void do_handler_debug(exception_frame_t* frame);
void do_handler_NMI(exception_frame_t * frame);
void do_handler_breakpoint(exception_frame_t * frame);
void do_handler_overflow(exception_frame_t * frame);
void do_handler_bound_range(exception_frame_t * frame);
void do_handler_invalid_opcode(exception_frame_t * frame);
void do_handler_device_unavailable(exception_frame_t * frame);
void do_handler_double_fault(exception_frame_t * frame);
void do_handler_invalid_tss(exception_frame_t * frame);
void do_handler_segment_not_present(exception_frame_t * frame);
void do_handler_stack_segment_fault(exception_frame_t * frame);
void do_handler_general_protection(exception_frame_t * frame);
void do_handler_page_fault(exception_frame_t * frame);
void do_handler_fpu_error(exception_frame_t * frame);
void do_handler_alignment_check(exception_frame_t * frame);
void do_handler_machine_check(exception_frame_t * frame);
void do_handler_smd_exception(exception_frame_t * frame);
void do_handler_virtual_exception(exception_frame_t * frame);

#endif // IRQ_H