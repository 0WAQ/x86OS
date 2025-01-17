#include "macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

int do_echo(int argc, char** argv) {
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