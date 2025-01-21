/**
 * 
 * 文件相关c文件 
 * 
 */
#include "fs/file.h"
#include "ipc/mutex.h"
#include "tools/klib.h"

// 系统文件表
static file_t sys_file_table[SYS_OFILE_NR];

// 访问file_table的互斥锁
static mutex_t mtx;

void file_table_init() {
    mutex_init(&mtx);
    kernel_memset(&sys_file_table, 0, sizeof(sys_file_table));
}

file_t* file_alloc() {
    file_t* file = NULL;
    mutex_lock(&mtx);

    // 遍历系统文件表, 寻找一个空块
    for(int i = 0; i < SYS_OFILE_NR; i++) {
        file_t* p = sys_file_table + i;
        if(p->ref == 0) {
            kernel_memset((void*)p, 0, sizeof(file_t));
            p->ref = 1;
            file = p;
            break;
        }
    }

    mutex_unlock(&mtx);
    return file;
}

void file_dec_ref(file_t* file) {
    mutex_lock(&mtx);
    if(file->ref != 0) {
        --file->ref;
    }
    mutex_unlock(&mtx);
}

void file_inc_ref(file_t* file) {
    mutex_lock(&mtx);
    file->ref++;
    mutex_unlock(&mtx);
}
