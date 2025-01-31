#include "sys/syscall.h"

int sys_unlink(const char* filename) {
    int ret = -1;
    if(root_fs->op->unlink != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->unlink(root_fs, filename);
        fs_unlock(root_fs);
    }
    return ret;
}