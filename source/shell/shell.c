/***
 * 
 * 命令行解释器
 * 
 */
#include "shell.h"
#include "cmd.h"
#include "lib_syscall.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>

cli_t cli;
static const char* promot = "sh >> ";

static cli_cmd_t cmd_table[] = {
    {
        .name = "help",
        .usage = "-- list supported commmand.",
        .func = do_help,
    },
    {
        .name = "clear",
        .usage = "-- clear screen",
        .func = do_clear,
    },
    {
        .name = "echo",
        .usage = "[-n count] msg -- echo something",
        .func = do_echo,
    },
    {
        .name = "ls",
        .usage = "ls -- list directory",
        .func = do_ls,
    },
    {
        .name = "less",
        .usage = "less [-l] file -- show file",
        .func = do_less,
    },
    {
        .name = "cp",
        .usage = "cp src dest",
        .func = do_cp,
    },
    {
        .name = "rm",
        .usage = "rm file -- remove file",
        .func = do_rm,
    },
    {
        .name = "quit",
        .usage = "quit from shell",
        .func = do_quit,
    }
};

int main(int argc, char** argv) {
    int fd = open(argv[0], O_RDWR);
    dup(fd);
    dup(fd);

    cli_init(promot, cmd_table, sizeof(cmd_table) / sizeof(cmd_table[0]));

    while(1) {
        show_pormot();
        char* str = fgets(cli.ibuf, CLI_IBUF_SIZE, stdin);
        if(!str) {
            continue;
        }

        char* p = strchr(cli.ibuf, '\r');
        while(*p) {
            *p++ = '\0';
        }

        int argc = 0;
        char* argv[CLI_ARG_MAX_NR];
        memset(argv, 0, CLI_ARG_MAX_NR);

        const char* delim = " ";
        char* token = strtok(cli.ibuf, delim);
        while(token) {
            argv[argc++] = (char*)token;
            token = strtok(NULL, delim);
        }

        if(argc == 0) {
            continue;
        }
        
        // 查找内置命令
        const cli_cmd_t* cmd = find_builtin(argv[0]);
        if(cmd) {
            run_builtin(cmd, argc, argv);
            continue;
        }

        // 从磁盘上执行
        const char* path = find_exec_file(argv[0]);
        if(path != NULL) {
            run_exec_file(path, argc, argv);
            continue;
        }

        fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli.ibuf);
    }
}

void cli_init(const char* promot, cli_cmd_t* cmd_table, int size) {
    cli.promot = promot;
    memset(cli.ibuf, 0, CLI_IBUF_SIZE);
    cli.cmd_start = cmd_table;
    cli.cmd_end = cmd_table + size;
}

static void show_pormot() {
    printf("%s", cli.promot);
    fflush(stdout);
}

static const cli_cmd_t* find_builtin(char* name) {
    for(const cli_cmd_t* cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if(strcmp(cmd->name, name) != 0) {
            continue;
        }

        return cmd;
    }

    return NULL;
}

static void run_builtin(const cli_cmd_t* cmd, int argc, char** argv) {
    int ret = cmd->func(argc, argv);
    if(ret < 0) {
        fprintf(stderr, ESC_COLOR_ERROR"Error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
}

static const char* find_exec_file(const char* filename) {
    static char path[255];
    int fd = open(filename, 0);
    if(fd < 0) {
        sprintf(path, "%s.elf", filename);
        fd = open(path, 0);
        if(fd < 0) {
            return NULL;
        }
        close(fd);
        return path;
    }

    close(fd);
    return filename;
}

static void run_exec_file(const char* path, int argc, char** argv) {
    int pid = fork();
    if(pid < 0) {
        fprintf(stderr, "fork failed %s", path);
    }
    else if(pid == 0) {
        int ret = execve(path, argv, NULL);
        if(ret < 0) {
            fprintf(stderr, "exec failed: %s\n", path);
        }
        exit(-1);
    }
    else {
        int status;
        int pid = wait(&status);
        fprintf(stderr, "cmd %s result: %d, pid: %d\n", path, status, pid);
    }
}