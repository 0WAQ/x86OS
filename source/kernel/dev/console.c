/***
 * 
 * 终端显示c文件
 * 
 */
#include "dev/console.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"

/**
 * @brief console表
 */
static console_t console_buf[CONSOLE_NR];


int console_init(int idx) {
    console_t* console = console_buf + idx;

    console->rows = CONSOLE_ROW_MAX;
    console->cols = CONSOLE_COL_MAX;

    // 设置颜色
    console->foreground = COLOR_WHITE;
    console->background = COLOR_BLACK;

    // 当前终端的起始地址
    console->disp_base = (disp_char_t*)CONSOLE_DISP_ADDR_START + 
                            idx * (CONSOLE_ROW_MAX * CONSOLE_COL_MAX);

    // 初始化光标
    cursor_t* cursor = &console->cursor;
    if(idx == 0) {
        uint16_t pos = read_cursor_pos();
        cursor->row = pos / console->cols;
        cursor->col = pos % console->cols;
    } else {
        cursor->row = cursor->col = 0;
        clear_display(console);
        update_cursor_pos(console);
    }

    // 初始化old光标
    cursor_t* old_cursor = &console->old_cursor;
    old_cursor->col = console->cols;
    old_cursor->row = console->rows;
    console->write_state = CONSOLE_WRITE_NORMAL;
    
    return 0;
}

int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    
    int len;
    for(len = 0; len < size; len++) {
        char ch = *data++;
        switch (c->write_state)
        {
        case CONSOLE_WRITE_NORMAL:
            write_normal(c, ch);
        break;
        
        case CONSOLE_WRITE_ESC:
            write_esc(c, ch);
        break;

        case CONSOLE_WRITE_SQUARE:
            write_esc_square(c, ch);
        break;

        default:
        break;
        }
    }
    update_cursor_pos(c);
    return len;
}

static int write_normal(console_t* c, char ch) {
    switch (ch)
    {
    case ASCII_ESC:
        c->write_state = CONSOLE_WRITE_ESC;
    break;

    case 0x7F:
        erase_left(c);
    break;

    case '\b':
        move_left(c, 1);
    break;

    case '\r':
        move_to_col0(c);
    break;

    case '\n':
        move_to_col0(c);  // 移动光标
        move_next_row(c); // 移动到下一行
    break;

    default:
        if(ch >= ' ' && ch <= '~') {
            show_char(c, ch);
        }
    break;
    }
}

static int write_esc(console_t* c, char ch) {
    switch (ch)
    {
    case '7':
        save_cursor(c);
        c->write_state = CONSOLE_WRITE_NORMAL;
    break;

    case '8':
        restore_cursor(c);
        c->write_state = CONSOLE_WRITE_NORMAL;
    break;
    
    case '[':
        // 清空esc参数缓冲区
        kernel_memset((void*)c->esc_param_buf, 0, sizeof(c->esc_param_buf));
        c->esc_param_index = 0;

        c->write_state = CONSOLE_WRITE_SQUARE;
    break;

    default:
        c->write_state = CONSOLE_WRITE_NORMAL;
    break;
    }
}

static void write_esc_square(console_t* c, char ch) {

    // 读取参数序列, 填充至缓冲区
    if((ch >= '0' && ch <= '9')) {
        uint32_t* param = &c->esc_param_buf[c->esc_param_index];
        *param = *param * 10 + (ch - '0');
    }
    // ;是参数的间隔符
    else if((ch == ';') && (c->esc_param_index < ESC_PARAM_MAX)) {
        c->esc_param_index++;
    }
    else {
        switch (ch)
        {
        case 'm':
            set_font_style(c);    
        break;
        
        case 'D':
            move_left(c, c->esc_param_buf[0]);
        break;

        case 'C':
            move_right(c, c->esc_param_buf[0]);
        break;

        case 'H':
        case 'f':
            move_cursor_esc(c);
        break;

        case 'J':
            erase_in_display(c);
        break;

        default:
        break;
        }

        c->write_state = CONSOLE_WRITE_NORMAL;
    }
}

void console_close(int console) {

}

static void clear_display(console_t* console) {
    uint32_t size = console->cols * console->rows;
    disp_char_t* p = console->disp_base;
    for(uint32_t i = 0; i < size; ++i, ++p) {
        p->ch = ' ';
        p->foreground = console->foreground;
        p->background = console->background;
    }
}

static void show_char(console_t* console, char ch) {
    
    // 获取当前终端的光标
    cursor_t cursor = console->cursor;
    
    // 计算偏移
    uint32_t offset = cursor.col + cursor.row * console->cols;

    // 
    disp_char_t* p = console->disp_base + offset;
    p->ch = ch;
    p->foreground = console->foreground;
    p->background = console->background;

    // 处理光标
    move_right(console, 1);
}

static void move_right(console_t* console, int n) {
    cursor_t* cursor = &console->cursor;
    for(int i = 0; i < n; i++) {
        
        // 列超过最大显示范围
        if(++cursor->col >= console->cols) {
            
            cursor->col = 0;

            // 若超过最大行数, 则上滚一行
            if(++cursor->row >= console->rows) {
                scroll_up(console, 1);
            }
        }
    }
}

static int move_left(console_t* console, int n) {
    int status = -1;
    cursor_t* cursor = &console->cursor;
    for(int i = 0; i < n; i++) {
        if(cursor->col > 0) {
            --cursor->col;
            status = 0;
        }
        else if(cursor->row > 0) {
            --cursor->row;
            cursor->col = console->cols - 1;
            status = 0;
        }
    }
    return status;
}

static void move_to_col0(console_t* console) {
    cursor_t* cursor = &console->cursor;
    cursor->col = 0; 
}

static void move_next_row(console_t* console) {
    cursor_t* cursor = &console->cursor;

    // 若超过最大行数, 则屏幕上滚一行
    if(++cursor->row >= console->rows) {
        scroll_up(console, 1);
    }
}

static void scroll_up(console_t* console, uint32_t lines) {
    disp_char_t* dest = console->disp_base;
    disp_char_t* src = console->disp_base + console->cols * lines;
    uint32_t size = (console->rows - lines) * console->cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    // 清空后几行
    erase_row(console, console->rows - lines, lines);

    cursor_t* cursor = &console->cursor;
    cursor->row -= lines;
}

static void erase_row(console_t* console, uint32_t start, uint32_t lines) {
    disp_char_t* dstart = console->disp_base + console->cols * start;
    disp_char_t* dend   = console->disp_base + console->cols * (start + lines);
    while(dstart < dend) {
        dstart->ch = ' ';
        dstart->foreground = console->foreground;
        dstart->background = console->background;

        ++dstart;
    }
}

static uint16_t read_cursor_pos() {
    uint16_t pos;
    outb(0x3D4, 0xF);
    pos = inb(0x3D5);
    outb(0x3D4, 0xE);
    pos |= inb(0x3D5) << 8;
    return pos;
}

static uint16_t update_cursor_pos(console_t* console) {
    cursor_t* cursor = &console->cursor;
    uint16_t pos = cursor->row * console->cols + cursor->col;

    outb(0x3D4, 0xF);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0xE);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    return pos;
}

static void erase_left(console_t* console) {
    // 向左移动一位, 打印空格覆盖掉原本的字符, 然后在向左移动
    if(move_left(console, 1) == 0) {
        show_char(console, ' ');
        move_left(console, 1);
    }
}

static inline void save_cursor(console_t* c) {
    c->old_cursor = c->cursor;
}

static inline void restore_cursor(console_t* c) {
    c->cursor = c->old_cursor;
}

static void set_font_style(console_t* c) {

    static const color_t color_table[] = {
        COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
        COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
    };

    // 从参数缓冲区取值
    for(int i = 0; i <= c->esc_param_index; i++) {
        uint32_t param = c->esc_param_buf[i];
        if((param >= 30) && (param <= 37)) {
            c->foreground = color_table[param - 30];
        }
        else if((param >= 40) && (param <= 47)) {
            c->background = color_table[param - 40];
        }
        else if(param == 39) {
            c->foreground = COLOR_WHITE;
        }
        else if(param == 49) {
            c->background = COLOR_BLACK;
        }
    }
}

static void erase_in_display(console_t* c) {
    if(c->esc_param_index < 0) {
        return;
    }

    uint32_t param = c->esc_param_buf[0];
    switch (param)
    {
    case 1:

    break;

    case 2:
        erase_row(c, 0, c->cols - 1);
        c->cursor.col = c->cursor.row = 0;
    break;
    
    case 3:

    break;

    default:
        break;
    }
}

static void move_cursor_esc(console_t* c) {
    cursor_t* cursor = &c->cursor;
    cursor->row = c->esc_param_buf[0];
    cursor->col = c->esc_param_buf[1];
}

static void move_left_esc(console_t* c, int n) {
    if(n == 0) {
        n = 1;
    }

    int col = c->cursor.col - n;
    c->cursor.col = (col >= 0) ? col : 0;
}

static void move_right_esc(console_t* c, int n) {
    if(n == 0) {
        n = 1;
    }

    int col = c->cursor.col + n;
    if(col >= c->cols) {
        c->cursor.col = c->cols - 1;
    }
    else {
        c->cursor.col = col;
    }
}