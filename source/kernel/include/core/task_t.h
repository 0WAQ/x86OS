/**
 * 
 * 
 * 
 */
#ifndef TASK_T_H
#define TASK_T_H

#include "common/types.h"
#include "cpu/cpu_t.h"

/**
 * 同Linux中的task_struct
 */
typedef struct _task_t {
    tss_t tss;
}task_t;

#endif // TASK_T_H