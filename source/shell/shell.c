/***
 * 
 * 命令行解释器
 * 
 */

#include "shell.h"
#include "lib_syscall.h"
#include "fs/file.h"
#include "dev/tty_t.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/file.h>

static cli_t cli;
static const char* promot = "sh >> ";

static int do_help(int argc, char** argv) {
    cli_cmd_t* start = (cli_cmd_t*)cli.cmd_start;
    while(start < cli.cmd_end) {
        printf("%s %s\n", start->name, start->usage);
        start++;
    }
    return 0;
}

static int do_clear(int argc, char** argv) {
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

static int do_echo(int argc, char** argv) {
    if(argc == 1) {
        char buf[128];
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf) - 1] = '\0';
        puts(buf);
        return 0;
    }

    int cnt = 1;
    char* opt = "n:h";

    int ch;
    while((ch = getopt(argc, argv, opt)) != -1) {
        switch (ch)
        {
        case 'h':
            puts("echo any message.");
            puts("Usage: echo [-n count] message.");
            optind = 1;

            return 0;

        case 'n':
            cnt = atoi(optarg);
            break;

        case '?':
            if(optarg) {
                fprintf(stderr, "Unknown option: -%s\n", optarg);
            }
            optind = 1;
            return -1;

        default:
            break;
        }
    }

    if(optind > argc - 1) {
        fprintf(stderr, "Message is Empty.\n");
        optind = 1;
        return -1;
    }

    char* msg = argv[optind];
    for(int i = 0; i < cnt; i++) {
        puts(msg);
    }
    optind = 1;
    return 0;
}

static int do_ls(int argc, char** argv) {
    // 打开目录
    DIR* p = opendir("temp");
    if(p == NULL) {
        printf("open dir failed.\n");
        return -1;
    }

    // 遍历目录项
    struct dirent* entry;
    while((entry = readdir(p)) != NULL) {
        strlwr(entry->name);
        printf("%c %s %d\n", (entry->type == FILE_DIR ? 'd': 'f'), entry->name, entry->size);
    }

    // 关闭目录
    closedir(p);
    return 0;
}

static int do_less(int argc, char** argv) {
    char* opt = "lh";

    int line_mode = 0;

    int ch;
    while((ch = getopt(argc, argv, opt)) != -1) {
        switch (ch)
        {
        case 'h':
            puts("show file content.");
            puts("Usage: less [-l] file.");
            optind = 1;
            return 0;
        case 'l':
            line_mode = 1;
            break;
        case '?':
            if(optarg) {
                fprintf(stderr, "Unknown option: -%s\n", optarg);
            }
            optind = 1;
            return -1;
        default:
            break;
        }
    }

    if(optind > argc - 1) {
        fprintf(stderr, "File Not Found.\n");
        optind = 1;
        return -1;
    }

    // 打开文件
    FILE* file = fopen(argv[optind], "r");
    if(file == NULL) {
        fprintf(stderr, "open file failed, %s\n", argv[optind]);
        optind = 1;
        return -1;
    }

    // 分配缓冲区, 将文件内容写入缓冲区
    char* buf = (char*)malloc(255);
    if(line_mode == 0) {
        while(fgets(buf, 255, file) != NULL) {
            fputs(buf, stdout);
        }
    }
    else {
        setvbuf(stdin, NULL, _IONBF, 0);
        ioctl(0, TTY_CMD_ECHO, 0, 0);
        while(1) {
            char* b = fgets(buf, 255, file);
            if(b == NULL) {
                break;
            }
            fputs(buf, stdout);  

            int ch;
            while((ch = fgetc(stdin)) != 'n') {
                if(ch == 'q') {
                    goto less_quit;
                }
            }
        }
less_quit:
        ioctl(0, TTY_CMD_ECHO, 1, 0);
        setvbuf(stdin, NULL, _IOLBF, BUFSIZ);
    }
    
    free(buf);
    fclose(file);
    optind = 1;
    return 0;
}

// TODO:
static int do_quit(int argc, char** argv) {
    exit(0);
    return 0;
}

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
    // TODO:  测试
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
            run_exec_file("", argc, argv);
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

void show_pormot() {
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
    int fd = open(filename, 0);
    if(fd < 0) {
        return NULL;
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