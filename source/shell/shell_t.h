/**
 * 
 * shell基本类型
 * 
 */
#ifndef SHELL_T_H
#define SHELL_T_H

// Command Line Interface
#define CLI_IBUF_SIZE           (1024)
#define CLI_ARG_MAX_NR          (10)

#define ESC_CMD2(Pn, cmd)               "\x1b["#Pn#cmd
#define ESC_CLEAR_SCREEN                ESC_CMD2(2, J)
#define ESC_COLOR_ERROR                 ESC_CMD2(31, m)
#define ESC_COLOR_DEFAULT               ESC_CMD2(39, m)
#define ESC_MOVE_CURSOR(ROW, COL)       "\x1b["#ROW";"#COL"H"

/**
 * @brief shell命令结构
 */
typedef struct _cli_cmd_t {
    const char* name;           // 命令名称
    const char* usage;          // 命令帮助信息
    int (*func)(int, char**);   // 命令执行函数
}cli_cmd_t;

/**
 * @brief shell结构
 */
typedef struct _cli_t {
    char ibuf[CLI_IBUF_SIZE];   // shell的输入缓冲区
    const cli_cmd_t* cmd_start; // shell命令表的起始地址
    const cli_cmd_t* cmd_end;
    const char* promot;         // 命令提示符
}cli_t;

#endif // SHELL_T_H