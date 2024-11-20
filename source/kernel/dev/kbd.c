/**
 * 
 * 键盘c文件
 * 
 */
#include "dev/kbd.h"
#include "cpu/irq.h"
#include "common/cpu_instr.h"
#include "tools/klib.h"
#include "tools/log.h"

/**
 * @brief
 */
static kbd_state_t kbd_stat;

/**
 * @brief 键值映射表
 */
static const key_map_t map_table[256] = {
        [0x02] = {'1', '!'},
        [0x03] = {'2', '@'},
        [0x04] = {'3', '#'},
        [0x05] = {'4', '$'},
        [0x06] = {'5', '%'},
        [0x07] = {'6', '^'},
        [0x08] = {'7', '&'},
        [0x09] = {'8', '*'},
        [0x0A] = {'9', '('},
        [0x0B] = {'0', ')'},
        [0x0C] = {'-', '_'},
        [0x0D] = {'=', '+'},
        [0x0E] = {'\b', '\b'},
        [0x0F] = {'\t', '\t'},
        [0x10] = {'q', 'Q'},
        [0x11] = {'w', 'W'},
        [0x12] = {'e', 'E'},
        [0x13] = {'r', 'R'},
        [0x14] = {'t', 'T'},
        [0x15] = {'y', 'Y'},
        [0x16] = {'u', 'U'},
        [0x17] = {'i', 'I'},
        [0x18] = {'o', 'O'},
        [0x19] = {'p', 'P'},
        [0x1A] = {'[', '{'},
        [0x1B] = {']', '}'},
        [0x1C] = {'\n', '\n'},
     // [0x1D] = lctrl / rctrl
        [0x1E] = {'a', 'A'},
        [0x1F] = {'s', 'B'},
        [0x20] = {'d', 'D'},
        [0x21] = {'f', 'F'},
        [0x22] = {'g', 'G'},
        [0x23] = {'h', 'H'},
        [0x24] = {'j', 'J'},
        [0x25] = {'k', 'K'},
        [0x26] = {'l', 'L'},
        [0x27] = {';', ':'},
        [0x28] = {'\'', '"'},
        [0x29] = {'`', '~'},
     // [0x2A] = lshift,
        [0x2B] = {'\\', '|'},
        [0x2C] = {'z', 'Z'},
        [0x2D] = {'x', 'X'},
        [0x2E] = {'c', 'C'},
        [0x2F] = {'v', 'V'},
        [0x30] = {'b', 'B'},
        [0x31] = {'n', 'N'},
        [0x32] = {'m', 'M'},
        [0x33] = {',', '<'},
        [0x34] = {'.', '>'},
     // [0x36] = rshift,
        [0x35] = {'/', '?'},
        [0x39] = {' ', ' '},
     // [0x3A] = CapsLock,
     // [0x3B] = F1,
     // [0x3C] = F2,
     // [0x3D] = F3,
     // [0x3E] = F4,
     // [0x3F] = F5,
     // [0x40] = F6,
     // [0x41] = F7,
     // [0x42] = F8,
     // [0x43] = F9,
     // [0x44] = F10,
     // [0x45] = NumberLock,
     // [0x46] = ScrollLock
        [0x47] = {'7', '7'},    // 以下为到0x53为keypad
        [0x48] = {'8', '8'},
        [0x49] = {'9', '9'},
        [0x4A] = {'-', '-'},
        [0x4B] = {'4', '4'},
        [0x4C] = {'5', '5'},
        [0x4D] = {'6', '6'},
        [0x4E] = {'+', '+'},
        [0x4F] = {'1', '1'},
        [0x50] = {'2', '2'},
        [0x51] = {'3', '3'},
        [0x52] = {'0', '0'},
        [0x53] = {'.', ','},
     // [0x57] = F11,
     // [0x58] = F12,
     // [0xE0] = rctrl
};

void kbd_init() {
    
    // 初始化led灯
    update_capslk_led_status();
    update_numlk_led_status();

    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}

void do_handler_kbd(exception_frame_t* frame) {

    static enum {
        NORMAL,
        BEGIN_E0,
        BEGIN_E1
    }recv_state = NORMAL;

    // 检查是否有数据, 没有则退出
    uint32_t status = inb(KBD_PORT_STAT);
    if(!(status & KBD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }

    // 读取键值
    uint8_t raw = inb(KBD_PORT_DATA);

    // 读取完成后发送EOI, 方便后续继续响应键盘中断
    pic_send_eoi(IRQ1_KEYBOARD);

    if(raw == KBD_KEY_E0) {
        recv_state = BEGIN_E0;
    }
    else if(raw == KBD_KEY_E1) {
        recv_state = BEGIN_E1;
    }
    else {
        switch (recv_state)
        {
        case NORMAL:
            do_normal_key(raw);
            break;
        case BEGIN_E0:
            do_e0_key(raw);
            recv_state = NORMAL;
        case BEGIN_E1:
            // TODO: do_e1_key(raw); // 只有pause键
            recv_state = NORMAL;
        }
    }
}

void do_e0_key(uint8_t raw) {
    char key = get_key(raw);
    int is_make = is_make_code(raw);

    switch (key)
    {
    case KBD_KEY_CTRL:
        kbd_stat.lctrl = is_make;
        break;
    
    case KBD_KEY_ALT:
        kbd_stat.lalt = is_make;
        break;
    }
}

void do_normal_key(uint8_t raw) {
    char key = get_key(raw);
    int is_make = is_make_code(raw);

    switch (key)
    {
    case KBD_KEY_LSHIFT:
        kbd_stat.lshift = is_make ? 1 : 0;
        break;

    case KBD_KEY_RSHIFT:
        kbd_stat.rshift = is_make ? 1 : 0;
        break;

    case KBD_KEY_CAPS:
        if(is_make) {
            kbd_stat.caps_lock = ~kbd_stat.caps_lock;
            update_capslk_led_status();
        }
        break;
    
    case KBD_KEY_NUM_LOCK:
        if(is_make) {
            kbd_stat.num_lock = ~kbd_stat.num_lock;
            update_numlk_led_status();
        }
        break;

    case KBD_KEY_ALT:
        kbd_stat.lalt = is_make;
        break;

    case KBD_KEY_CTRL:
        kbd_stat.lctrl = is_make;
        break;

    // 功能键：写入键盘缓冲区，由应用自行决定如何处理
    case KBD_KEY_F1:
    case KBD_KEY_F2:
    case KBD_KEY_F3:
    case KBD_KEY_F4:
    case KBD_KEY_F5:
    case KBD_KEY_F6:
    case KBD_KEY_F7:
    case KBD_KEY_F8:
    case KBD_KEY_F9:
    case KBD_KEY_F10:
    case KBD_KEY_F11:
    case KBD_KEY_F12:
    case KBD_KEY_SCROLL_LOCK:
        break;

    default:

        if(is_make) {

            // 根据shift键是否被按下, 判断使用哪个功能
            if(kbd_stat.lshift || kbd_stat.rshift) {
                key = map_table[key].func;
            }
            else {
                key = map_table[key].normal;
            }

            // 判断大写锁定是否开启, 若开启, 则大写变小写, 小写变大写
            if(kbd_stat.caps_lock) {
                if((key >= 'A') && (key <= 'Z')) {
                    key = key - 'A' + 'a';
                }
                else if((key >= 'a') && (key <= 'z')) {
                    key = key - 'a' + 'A';
                }
            }

            // TODO: 暂无逻辑
            if(kbd_stat.num_lock) {

            }
            else {

            }

            log_print("key: %c", key); // TODO: 待删除
        }

        break;
    }
}

static void update_capslk_led_status() {
    int data = (kbd_stat.caps_lock ? 1 : 0) << 1;

    // 写入data
    kbd_write(KBD_PORT_DATA, KBD_CMD_RW_LED);
    kbd_write(KBD_PORT_DATA, data);
    kbd_read();
}

static void update_numlk_led_status() {
    int data = (kbd_stat.num_lock ? 1 : 0) << 2;

    // 写入data
    kbd_write(KBD_PORT_DATA, KBD_CMD_RW_LED);
    kbd_write(KBD_PORT_DATA, data);
    kbd_read();
}

uint8_t kbd_read() {
    kbd_wait_recv_ready();
    return inb(KBD_PORT_DATA);
}

void kbd_write(uint8_t port, uint8_t data) {
    kbd_wait_send_ready();
    outb(port, data);
}

void kbd_wait_send_ready() {
    uint32_t timeout = 100000;
    while(timeout--) {
        if((inb(KBD_PORT_STAT) & KBD_STAT_SEND_FULL) == 0) {
            return;
        }
    }
}

void kbd_wait_recv_ready() {
    uint32_t timeout = 100000;
    while(timeout--) {
        if((inb(KBD_PORT_STAT) & KBD_STAT_RECV_READY) == 0) {
            return;
        }
    }   
}