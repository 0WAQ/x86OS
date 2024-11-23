/**
 * 
 * 文件相关c文件 
 * 
 */
#include "fs/file.h"
#include "ipc/mutex.h"
#include "tools/klib.h"

static file_t file_table[FILE_TABLE_NR];

static mutex_t mtx_file_alloc;

void file_table_init() {
    mutex_init(&mtx_file_alloc);
    kernel_memset((void*)file_table, 0, sizeof(file_table));
}

file_t* file_alloc() {
    file_t* file = NULL;

    mutex_lock(&mtx_file_alloc);

    for(int i = 0; i < FILE_TABLE_NR; i++) {
        file_t* p = file_table + i;
        if(p->ref == 0) {
            kernel_memset((void*)p, 0, sizeof(file_t));
            p->ref = 1;
            file = p;
            break;
        }
    }

    mutex_unlock(&mtx_file_alloc);

    return file;
}

void file_free(file_t* file) {
    mutex_lock(&mtx_file_alloc);

    if(file->ref != 0) {
        --file->ref;
    }

    mutex_unlock(&mtx_file_alloc);
}
