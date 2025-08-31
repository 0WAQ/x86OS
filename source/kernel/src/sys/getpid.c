#include "sys/syscall.h"

int sys_getpid() {
    return get_curr_task()->pid;
}