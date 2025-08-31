#include "sys/syscall.h"

int sys_readdir(DIR* dir, struct dirent* dirent) {
    int ret = -1;
    if(root_fs->op->readdir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->readdir(root_fs, dir, dirent);
        fs_unlock(root_fs);
    }
    return ret;
}