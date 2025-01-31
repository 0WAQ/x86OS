#include "sys/syscall.h"

int sys_closedir(DIR* dir) {
    int ret = -1;
    if(root_fs->op->closedir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->closedir(root_fs, dir);
        fs_unlock(root_fs);
    }
    return ret;
}