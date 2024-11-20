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
 * @brief
 */
static void move_to_col0(console_t* console);

/**
 * @brief
 */
static void move_next_row(console_t* console);

/**
 * @brief
 */
static void scroll_up(console_t* console, uint32_t lines);

/**
 * @brief
 */
static void erase_row(console_t* console, uint32_t start, uint32_t lines);

/**
 * @brief
 */
static uint16_t read_cursor_pos();

/**
 * @brief
 */
static uint16_t update_cursor_pos(console_t* console);

#endif // CONSOLE_H
