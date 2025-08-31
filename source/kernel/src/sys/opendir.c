#include "sys/syscall.h"

int sys_opendir(const char* path, DIR* dir) {
    int ret = -1;
    if(root_fs->op->opendir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->opendir(root_fs, path, dir);
        fs_unlock(root_fs);
    }
    return ret;
}