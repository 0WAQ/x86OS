/**
 * 
 * 文件系统C文件
 * 
 */
#include "fs/fs.h"
#include "fs/file.h"
#include "dev/dev.h"
#include "dev/console.h"
#include "dev/disk.h"
#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/list.h"
#include "common/cpu_instr.h"
#include "os_cfg.h"
#include <sys/stat.h>
#include <sys/file.h>

static list_t mounted_list;             // 挂载链表
static list_t free_list;                // 空闲链表
static fs_t fs_table[FS_TABLE_SIZE];    // fs表
static fs_t* root_fs;                   // 根文件系统

extern fs_op_t devfs_op;                // TODO: 临时的
extern fs_op_t fat16fs_op;              // FAT16

void fs_init() {

    // 初始化文件系统的链表
    mount_list_init();

    // 初始化文件表
    file_table_init();

    // 初始化磁盘
    disk_init();

    // 挂载devfs
    fs_t* fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != NULL);

    // 挂载根文件系统
    root_fs = mount(FS_FAT16, "/home", ROOT_DEV);
    ASSERT(root_fs != NULL);
}

// TODO: TEMP
static int is_path_valid(const char* path) {
    if((path == NULL) || (path[0] == '\0')) {
        return 0;
    }
    return 1;
}

int sys_open(const char* filename, int flags, ...) {
    // 分配文件描述符链接
    file_t* file = file_alloc();
    if(file == NULL) {
        goto sys_open_failed;
    }

    // 分配文件描述符
    int fd = task_alloc_fd(file);
    if(fd < 0) {
        goto sys_open_failed;
    }

    // 检查名称是否以挂载点开头, 若没有, 则认为filename在根目录下
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

    fs_lock(fs);
    int err = fs->op->open(fs, filename, file);
    if(err < 0) {
        fs_unlock(fs);
        log_print("open %s failed.", filename);
        goto sys_open_failed;
    }
    fs_unlock(fs);
    return fd;

sys_open_failed:
    if(file) {
        file_free(file);
    }
    if(fd >= 0) {
        task_remove_fd(fd);
    }
    return -1;
}

int sys_read(int fd, char* buf, int len) {
    if(is_fd_bad(fd) || buf == NULL || len == 0) {
        return 0;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    // 若文件是只写的
    if(p->mode == O_WRONLY) {
        log_print("file is write only");
        return -1;
    }

    fs_t* fs = p->fs;
    
    fs_lock(fs);
    int ret = fs->op->read(buf, len, p);
    fs_unlock(fs);
    return ret;
}

int sys_write(int fd, char* buf, int len) {
    if(is_fd_bad(fd) || buf == NULL || len == 0) {
        return 0;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    // 若文件是只写的
    if(p->mode == O_RDONLY) {
        log_print("file is write only");
        return -1;
    }

    fs_t* fs = p->fs;
    
    fs_lock(fs);
    int ret = fs->op->write(buf, len, p);
    fs_unlock(fs);
    return ret;
}

int sys_lseek(int fd, int offset, int dir) {
    // TODO:
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    fs_t* fs = p->fs;
    fs_lock(fs);
    int ret = fs->op->seek(p, offset, dir);
    fs_unlock(fs);
    return ret;
}

int sys_close(int fd) {
    // TODO: 

    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }

    // 要打开文件, 那么断言文件是打开的
    ASSERT(p->ref > 0);

    // 减少ref
    if(--p->ref == 0) {
        fs_t* fs = p->fs;
        fs_lock(fs);
        fs->op->close(p);
        fs_unlock(fs);

        file_free(p);
    }
    task_remove_fd(fd);
    return 0;
}

int sys_isatty(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }
    return p->type == FILE_TTY;
}

int sys_fstat(int fd, struct stat* st) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }

    fs_t* fs = p->fs;
    kernel_memset(st, 0, sizeof(struct stat));

    fs_lock(fs);
    int ret = fs->op->stat(p, st);
    fs_unlock(fs);
    return ret;
}

int sys_ioctl(int fd, int cmd, int arg0, int arg1) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(!p) {
        log_print("file not opened.");
        return -1;
    }

    fs_t* fs = p->fs;
    int ret = -1;
    fs_lock(fs);
    if(fs->op->ioctl != NULL) {
        ret = fs->op->ioctl(p, cmd, arg0, arg1);
    }
    fs_unlock(fs);
    return ret;
}

int sys_dup(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(!p) {
        log_print("file not opened.");
        return -1;
    }

    // 新创建一个fd与p相同
    int new = task_alloc_fd(p);
    if(new < 0) {
        log_print("no task file avaliable.");
        return -1;
    }
    
    file_inc_ref(p);
    return new;
}

int sys_opendir(const char* path, DIR* dir) {
    fs_lock(root_fs);
    int ret = root_fs->op->opendir(root_fs, path, dir);
    fs_unlock(root_fs);
    return ret;
}

int sys_readdir(DIR* dir, struct dirent* dirent) {
    fs_lock(root_fs);
    int ret = root_fs->op->readdir(root_fs, dir, dirent);
    fs_unlock(root_fs);
    return ret;
}

int sys_closedir(DIR* dir) {
    fs_lock(root_fs);
    int ret = root_fs->op->closedir(root_fs, dir);
    fs_unlock(root_fs);
    return ret;
}

static void mount_list_init() {
    
    // 初始化空闲链表
    list_init(&free_list);
    for(int i = 0; i < FS_TABLE_SIZE; i++) {
        list_insert_first(&free_list, &fs_table[i].node);
    }

    // 初始化挂载链表
    list_init(&mounted_list);
}

static fs_op_t* get_fs_op(fs_type_t type, int major) {
    switch (type)
    {
    case FS_DEVFS:
        return &devfs_op;
    case FS_FAT16:
        return &fat16fs_op;
    default:
        return NULL;
    }
    return NULL;
}

static fs_t* mount(fs_type_t type, char* mount_point, int dev_major, int dev_minor) {
    fs_t* fs = NULL;
    log_print("mount file system, name: %s, dev: %x,", mount_point, dev_major);

    // 判断当前文件是否挂载过
    list_node_t* curr = list_first(&mounted_list);      // 遍历挂载链表
    while(curr != NULL) {
        fs_t* p = list_entry_of(curr, fs_t, node);
        if(kernel_strncmp(fs->mount_point, mount_point, FS_MOUNTP_SIZE) == 0) {
            log_print("fs already mounted.");
            goto mount_failed;
        }
        curr = list_node_next(curr);
    }

    // 从空闲链表中分配新的fs
    list_node_t* free_node = list_remove_first(&free_list);
    if(free_node == NULL) {
        log_print("no free fs, mount failed.");
        goto mount_failed;
    }
    fs = list_entry_of(free_node, fs_t, node);

    // 获取当前fs的操作函数
    fs_op_t* op = get_fs_op(type, dev_major);
    if(op == NULL) {
        log_print("unsupport fs type: %d", type);
        goto mount_failed;
    }

    // 初始化fs
    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strncpy(fs->mount_point, mount_point, FS_MOUNTP_SIZE);
    fs->op = op;
    fs->mtx = NULL;

    // 执行挂载
    if(op->mount(fs, dev_major, dev_minor) < 0) {
        log_print("mount fs %s failed.", mount_point);
        goto mount_failed;
    }

    // 将节点移至挂载链表中
    list_insert_last(&mounted_list, &fs->node);
    return fs;

mount_failed:
    // 回收fs
    if(fs) {
        list_insert_last(&free_list, &fs->node);
    }
    return NULL;
}


const char* path_next_child(const char* path) {
    const char* p = path;
    while(*p && (*p++ == '/'));
    while(*p && (*p++ != '/'));
    return *p ? p : NULL;
}

int path_to_num(const char* path, int* num) {
    int x = 0;

    const char* p = path;
    while(*p) {
        x = x * 10 + (*p - '0');
        p++;
    }
    *num = x;
    return 0;
}

int path_begin_with(const char* path, const char* str) {
    const char* i = path, *j = str;
    while(*i && *j && (*i == *j)) {
        *i++;
        *j++;
    }
    return *j == '\0';
}

static int is_fd_bad(int fd) {
    return (fd < 0) || (fd >= TASK_OFILE_NR);
}

static void fs_lock(fs_t* fs) {
    if(fs->mtx) {
        mutex_lock(fs->mtx);
    }
}

static void fs_unlock(fs_t* fs) {
    if(fs->mtx) {
        mutex_unlock(fs->mtx);
    }
}