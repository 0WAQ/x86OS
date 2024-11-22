/***
 * 
 * 终端显示基本类型 
 * 
 */
#ifndef CONSOLE_T_H
#define CONSOLE_T_H

#include "common/types.h"

#define CONSOLE_DISP_ADDR_START     (0xB8000)
#define CONSOLE_DISP_ADDR_END       (0xB8000 + 32 * 1024)    

#define CONSOLE_ROW_MAX             (25)
#define CONSOLE_COL_MAX             (80)

#define CONSOLE_NR                  (8)

#define ASCII_ESC                   0x1B    // \033
#define ESC_PARAM_MAX               (10)

// 每个字符使用16位控制. 低8位表示字符; 高8位是属性, 其中低4位是前景色, 4~6位是背景色, // TODO:
typedef union _disp_char_t {
    uint16_t v;
    struct {
        char ch;
        char foreground : 4;    // 前景色
        char background : 3;    // 背景色
    };
}disp_char_t;

/**
 * @brief 颜色
 */
typedef enum {
    COLOR_BLACK             = 0,
    COLOR_BLUE              = 1,
    COLOR_GREEN             = 2,
    COLOR_CYAN              = 3,
    COLOR_RED               = 4,
    COLOR_MAGENTA           = 5,
    COLOR_BROWN             = 6,
    COLOR_GRAY              = 7,
    COLOR_DARKGRAY          = 8,
    COLOR_LIGHT_BLUE        = 9,
    COLOR_LIGHT_GREEN       = 10,
    COLOR_LIGHT_CYAN        = 11,
    COLOR_LIGHT_RED         = 12,
    COLOR_LIGHT_MAGENTA     = 13,
    COLOR_YELLOW            = 14,
    COLOR_WHITE             = 15,
}color_t;

/**
 * @brief 光标
 */
typedef struct _cursor_t {
    uint32_t row, col;
}cursor_t;

/**
 * @brief 终端结构体
 */
typedef struct _console_t {

    enum {
        CONSOLE_WRITE_NORMAL,
        CONSOLE_WRITE_ESC,
        CONSOLE_WRITE_SQUARE,
    }write_state;

    disp_char_t* disp_base;
    uint32_t rows, cols;
    color_t foreground, background;    // 当前终端的前景色和背景色

    cursor_t cursor;        // 光标
    
    // 保存光标的位置, 与ESC 7,8 有关
    cursor_t old_cursor;    

    // esc序列的参数缓冲区
    uint32_t esc_param_buf[ESC_PARAM_MAX];
    uint32_t esc_param_index;

}console_t;

#endif // CONSOLE_T_H