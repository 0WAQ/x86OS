/**
 * shell内置命令
 */
#ifndef CMD_H
#define CMD_H

#include "shell.h"
#include "applib/lib_syscall.h"

int do_help(int argc, char** argv);
int do_clear(int argc, char** argv);
int do_echo(int argc, char** argv);
int do_ls(int argc, char** argv);
int do_less(int argc, char** argv);
int do_cp(int argc, char** argv);
int do_rm(int argc, char** argv);
int do_date(int argc, char** argv);
int do_quit(int argc, char** argv);

#endif // CMD_H

#ifdef SHELL_H
#endif
#ifdef LIB_SYSCALL_H
#endif