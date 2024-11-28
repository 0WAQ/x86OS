/**
 * 
 * shell相关头文件
 * 
 */
#ifndef SHELL_H
#define SHELL_H

#include "shell_t.h"

/**
 * @brief 初始化命令行接口
 */
static void cli_init(const char* promot, cli_cmd_t* cmd_table, int size);

/**
 * @brief 答应命令提示符
 */
static void show_pormot();

/**
 * @brief 寻找内置命令
 */
static const cli_cmd_t* find_builtin(char* name);

/**
 * @brief 执行内置命令
 */
static void run_builtin(const cli_cmd_t* cmd, int argc, char** argv);

/**
 * @brief 执行磁盘上的命令
 */
static void run_exec_file(const char* path, int argc, char** argv);

#endif // SHELL_H