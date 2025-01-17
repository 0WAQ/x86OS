#include "macros.h"
#include <stdio.h>

int do_rm(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "no file.\n");
        return -1;
    }

    int ret = unlink(argv[1]);
    if(ret < 0) {
        fprintf(stderr, "rm file failed: %s.\n", argv[1]);
        return -1;
    }

    return 0;
}