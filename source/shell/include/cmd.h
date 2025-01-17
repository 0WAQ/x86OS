/**
 * 
 * shell内置命令头文件
 * 
 */
#ifndef CMD_H
#define CMD_H

/**
 * @brief help
 */
int do_help(int argc, char** argv);

/**
 * @brief clear
 */
int do_clear(int argc, char** argv);

/**
 * @brief echo
 */
int do_echo(int argc, char** argv);

/**
 * @brief ls
 */
int do_ls(int argc, char** argv);

/**
 * @brief less
 */
int do_less(int argc, char** argv);

/**
 * @brief cp
 */
int do_cp(int argc, char** argv);

/**
 * @brief rm
 */
int do_rm(int argc, char** argv);

/**
 * @brief quit
 */
int do_quit(int argc, char** argv);

#endif // CMD_H