/**
 * 
 * 键盘相关头文件
 * 
 */
#ifndef KBD_H
#define KBD_H

#include "kbd_t.h"
#include "cpu/irq_t.h"

/**
 * @brief 初始化键盘
 */
void kbd_init();

/**
 * @brief 键盘中断的处理程序
 */
void do_handler_kbd(exception_frame_t* frame);
void exception_handler_kbd();

/**
 * @brief 以E0开头的键值
 */
void do_e0_key(uint8_t raw);

/**
 * @brief 普通的键值
 */
void do_normal_key(uint8_t raw);

/**
 * @brief 切换tty设备
 */
void do_fx_key(char key);

/**
 * @brief 更新Caps Lock键的led灯
 */
static void update_capslk_led_status();

/**
 * @brief 更新Num Lock键的led灯
 */
static void update_numlk_led_status();

/**
 * @brief 判断键值是否是make_code(即按下)
 */
static inline int is_make_code(uint8_t raw) {
    return !(raw & 0x80);
}

/**
 * @brief 根据raw获取键值映射表中的键
 */
static inline char get_key(uint8_t raw) {
    return raw & 0x7F;
}

/**
 * @brief 读键盘数据
 */
uint8_t kbd_read();

/**
 * @brief 向键盘端口写数据
 */
void kbd_write(uint8_t port, uint8_t data);

/**
 * @brief 等待可写数据
 */
void kbd_wait_send_ready();

/**
 * @brief 等待可用的键盘数据
 */
void kbd_wait_recv_ready();

#endif // KBD_H