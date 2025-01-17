#include "macros.h"
#include <stdio.h>
#include <stdlib.h>

int do_cp(int argc, char** argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: cp src dest\n");
        return -1;
    }

    FILE* from, *to;
    from = fopen(argv[1], "rb");
    to = fopen(argv[2], "wb");
    if(from == NULL || to == NULL) {
        fprintf(stderr, "open file failed.\n");
        goto do_cp_failed;
    }

    char* buf = (char*)malloc(255);
    int len = 0;
    while((len = fread(buf, 1, 255, from)) > 0) {
        fwrite(buf, 1, len, to);
    }
    free(buf);

do_cp_failed:
    if(from) {
        fclose(from);
    }
    if(to) {
        fclose(to);
    }
    return 0;
}