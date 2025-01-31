/**
 * 
 * 硬件定时器配置
 * 
 */
#include "dev/timer.h"
#include "dev/dev.h"
#include "common/cpu_instr.h"
#include "os_cfg.h"
#include "core/task.h"

/**
 * @brief timer的设备层回调函数
 */
dev_desc_t dev_timer_desc = {
    .name = "timer",
    .major = DEV_TIMER,
    .open = NULL,
    .read = NULL,
    .write = NULL,
    .control = NULL,
    .close = NULL
};


static u32_t sys_tick; // 系统启动后的tick数量

void timer_init() {
    sys_tick = 0;
    pit_init();
}

static void pit_init() {
    u32_t reload_count = PIT_OSC_FREQ * OS_TICK_MS / 1000.0;

    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNEL0 | PIT_LOAD_LOHI | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);          // 加载低8位
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF);   // 加载高8位

    irq_install(IRQ0_TIMER, GATE_ATTR_DPL0, exception_handler_timer);
    irq_enable(IRQ0_TIMER);
}

void do_handler_timer(exception_frame_t* frame) {
    ++sys_tick;
    
    // 先调用pic_send_eoi, 而不是放在最后
    // 放最后将从任务中切换出去，除非任务再切换回来才能继续响应
    pic_send_eoi(IRQ0_TIMER);

    task_time_tick();
}