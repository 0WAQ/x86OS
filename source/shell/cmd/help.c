#include "macros.h"
#include <stdio.h>

extern cli_t cli;

int do_help(int argc, char** argv) {
    cli_cmd_t* start = (cli_cmd_t*)cli.cmd_start;
    while(start < cli.cmd_end) {
        printf("%s %s\n", start->name, start->usage);
        start++;
    }
    return 0;
}