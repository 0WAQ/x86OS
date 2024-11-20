/***
 * 
 * 终端显示c文件
 * 
 */
#include "dev/console.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"

/**
 * @brief
 */
static console_t console_buf[CONSOLE_NR];


int console_init() {
    for(int i = 0; i < CONSOLE_NR; i++) {
        console_t* console = console_buf + i;

        console->rows = CONSOLE_ROW_MAX;
        console->cols = CONSOLE_COL_MAX;

        // 初始化光标
        cursor_t* cursor = &console->cursor;
        uint16_t pos = read_cursor_pos();
        cursor->row = pos / console->cols;
        cursor->col = pos % console->cols;

        // 设置颜色
        console->foreground = COLOR_WHITE;
        console->background = COLOR_BLACK;

        // 当前终端的起始地址
        console->disp_base = (disp_char_t*)CONSOLE_DISP_ADDR_START + 
                                i * (CONSOLE_ROW_MAX * CONSOLE_COL_MAX);

        // 清空当前终端
        // clear_display(console);
    }
    return 0;
}

int console_write(int console, char* data, int size) {
    console_t* c = console_buf + console;
    
    int len;
    for(len = 0; len < size; len++) {
        char ch = *data++;
        switch (ch)
        {
        case '\n':
            move_to_col0(c);  // 移动光标
            move_next_row(c); // 移动到下一行
        break;

        default:
            show_char(c, ch);    
        }
    }
    update_cursor_pos(c);
    return len;
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