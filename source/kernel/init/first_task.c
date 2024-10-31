/**
 * 
 * first_task相关c文件
 * 
 */

#include "tools/log.h"
#include "core/task.h"

void first_task_main() {
    for(;;) {
        log_print("first task.");
        sys_sleep(1000);
    }
}
