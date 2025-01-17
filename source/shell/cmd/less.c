#include "macros.h"
#include "dev/tty.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int do_less(int argc, char** argv) {
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