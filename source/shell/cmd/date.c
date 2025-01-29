#include "macros.h"
#include <stdio.h>
#include <time.h>

int do_date(int argc, char** argv) {
    time_t t = time(NULL);
    fprintf(stdout, "%s", ctime(&t));
    return 0;
}