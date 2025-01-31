#include "sys/syscall.h"

// TODO: 若重复打开同一个文件, 会不会多在sys_file_alloc多分配一块?
int sys_open(const char* filename, int flags, ...) {
    // 再系统文件表中分配一个文件
    file_t* file = file_alloc();
    if(file == NULL) {
        goto sys_open_failed;
    }

    // 在任务中分配文件描述符
    int fd = task_alloc_fd(file);
    if(fd < 0) {
        goto sys_open_failed;
    }

    // 检查名称是否以挂载点开头(解析file对应的fs)
    // 若fs为空, 则认为filename在根目录下
    fs_t* fs = NULL;
    list_node_t* node = list_first(&mounted_list);
    while(node) {
        fs_t* curr = list_entry_of(node, fs_t, node);
        if(path_begin_with(filename, curr->mount_point)) {
            fs = curr;
            break;
        }
        node = list_node_next(node);
    }

    if(fs) {
        filename = path_next_child(filename);    
    }
    else {
        fs = root_fs;
    }

    file->mode = flags;
    file->fs = fs;
    kernel_strncpy(file->filename, filename, FILENAME_SIZE);

    if(fs->op->open != NULL) {
        fs_lock(fs);
        int ret = fs->op->open(fs, filename, file);
        if(ret < 0) {
            fs_unlock(fs);
            goto sys_open_failed;
        }
        fs_unlock(fs);
    }
    return fd;

sys_open_failed:
    if(file) {
        file_dec_ref(file);
    }
    if(fd >= 0) {
        task_remove_fd(fd);
    }
    return -1;
}