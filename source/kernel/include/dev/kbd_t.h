/**
 * 
 * 键盘基本类型
 * 
 */
#ifndef KBD_T_H
#define KBD_T_H

#include "common/types.h"

#define KBD_PORT_DATA           (0x60)
#define KBD_PORT_STAT           (0x64)
#define KBD_PORT_CMD            (0x64)

#define KBD_STAT_RECV_READY     (1 << 0)
#define KBD_STAT_SEND_FULL      (1 << 1)

// https://wiki.osdev.org/PS/2_Keyboard
#define KBD_CMD_RW_LED			(0xED)  // 写按键
#define KBD_KEY_E0			    (0xE0)	// E0编码
#define KBD_KEY_E1			    (0xE1)	// E1编码
#define	KBD_ASCII_ESC		    (0x1B)
#define	KBD_ASCII_DEL		    (0x7F)

// 特殊功能键
#define KBD_KEY_CTRL            (0x1D)  // E0或1D
#define KBD_KEY_LSHIFT          (0x2A)
#define KBD_KEY_RSHIFT          (0x36)
#define KBD_KEY_CAPS            (0x3A)
#define KBD_KEY_ALT 		    (0x38)  // E0或39

#define	KBD_KEY_FUNC		    (0x8000)
#define KBD_KEY_F1			    (0x3B)
#define KBD_KEY_F2			    (0x3C)
#define KBD_KEY_F3			    (0x3D)
#define KBD_KEY_F4			    (0x3E)
#define KBD_KEY_F5			    (0x3F)
#define KBD_KEY_F6			    (0x40)
#define KBD_KEY_F7			    (0x41)
#define KBD_KEY_F8			    (0x42)
#define KBD_KEY_F9			    (0x43)
#define KBD_KEY_F10			    (0x44)
#define KBD_KEY_F11			    (0x57)
#define KBD_KEY_F12			    (0x58)

#define KBD_KEY_NUM_LOCK        (0x45)
#define	KBD_KEY_SCROLL_LOCK		(0x46)          // 未使用
#define KBD_KEY_HOME			(0x47)
#define KBD_KEY_END				(0x4F)
#define KBD_KEY_PAGE_UP			(0x49)
#define KBD_KEY_PAGE_DOWN		(0x51)
#define KBD_KEY_CURSOR_UP		(0x48)
#define KBD_KEY_CURSOR_DOWN		(0x50)
#define KBD_KEY_CURSOR_RIGHT	(0x4D)
#define KBD_KEY_CURSOR_LEFT		(0x4B)
#define KBD_KEY_INSERT			(0x52)
#define KBD_KEY_DELETE			(0x53)
#define KBD_KEY_BACKSPACE		(0x0E)          // 在映射表中设置

/**
 * @brief 键值映射结构
 * 每个按键最多两个功能
 * 1. 无shift按下或者numlock灯亮的值, 即默认值
 * 2. shift按下或者numLock等灭的值, 为附加功能值
 */
typedef struct _key_map_t {
    uint8_t normal;     // 普通功能
    uint8_t func;       // 第二功能
}key_map_t;

/**
 * @brief 状态指示灯
 */
typedef struct _kbd_state_t {
    int caps_lock    : 1;   // caps_lock亮为1
    int num_lock     : 1;   // num_lock亮为1
    int lshift       : 1;   // 左shift
    int rshift       : 1;   // 右shift
    int lalt         : 1;   // 左alt
    int ralt         : 1;   // 右alt
    int lctrl        : 1;   // 左ctrl
    int rctrl        : 1;   // 右ctrl
}kbd_state_t;

#endif // KBD_T_H