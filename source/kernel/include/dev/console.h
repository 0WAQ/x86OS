/***
 * 
 * 终端显示头文件 
 * 
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "console_t.h"

/**
 * @brief 初始化终端
 */
int console_init();

/**
 * @brief 向终端写数据
 */
int console_write(int console, char* data, int size);

/**
 * @brief 写普通数据
 */
static int write_normal(console_t* c, char ch);

/**
 * @brief 写esc序列
 */
static int write_esc(console_t* c, char ch);

/**
 * @brief 写esc序列的 [ 参数列表
 */
static void write_esc_square(console_t* c, char ch);

/**
 * @brief 关闭终端
 */
void console_close(int console);

/**
 * @brief 清空屏幕
 */
static void clear_display(console_t* console);

/**
 * @brief 打印一个字符
 */
static void show_char(console_t* console, char ch);

/**
 * @brief 向右移动光标
 */
static void move_right(console_t* console, int n);

/**
 * @brief 左移动光标
 */
static int move_left(console_t* console, int n);

/**
 * @brief 光标移指第0列
 */
static void move_to_col0(console_t* console);

/**
 * @brief 光标移指下一行开头
 */
static void move_next_row(console_t* console);

/**
 * @brief 屏幕上滚lines行
 */
static void scroll_up(console_t* console, uint32_t lines);

/**
 * @brief 删除lines行
 */
static void erase_row(console_t* console, uint32_t start, uint32_t lines);

/**
 * @brief 光标向右移动,并删除字符
 */
static void erase_left(console_t* console);

/**
 * @brief 读取光标的位置
 */
static uint16_t read_cursor_pos();

/**
 * @brief 修改光标的位置
 */
static uint16_t update_cursor_pos(console_t* console);

/**
 * @brief 保存光标位置
 */
static inline void save_cursor(console_t* c);

/**
 * @brief 恢复光标位置
 */
static inline void restore_cursor(console_t* c);

/**
 * @brief esc [p0;p1;...m
 */
static void set_font_style(console_t* c);

/**
 * @brief esc序列使用的删除操作
 */
static void erase_in_display(console_t* c);

/**
 * @brief esc序列使用的移动光标
 */
static void move_cursor_esc(console_t* c);

/**
 * @brief esc序列使用的向左移动
 */
static void move_left_esc(console_t* c, int n);

/**
 * @brief esc序列使用的向右移动
 */
static void move_right_esc(console_t* c, int n);

#endif // CONSOLE_H
