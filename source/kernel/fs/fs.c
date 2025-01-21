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
static fs_t fs_table[FS_TABLE_SIZE];    // 文件系统表, 最多能挂载10个文件系统
                                        // 初始化阶段将其组织为空闲链表
static fs_t* root_fs;                   // 根文件系统

// 文件系统回调结构
extern fs_op_t devfs_op;                // DEV
extern fs_op_t fat16fs_op;              // FAT16

void fs_init() {

    // 初始化文件系统的链表
    mount_list_init();

    // 初始化系统文件表
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

int sys_unlink(const char* filename) {
    int ret = -1;
    if(root_fs->op->unlink != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->unlink(root_fs, filename);
        fs_unlock(root_fs);
    }
    return ret;
}

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
            log_print("open %s failed.", filename);
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

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->read != NULL) {
        fs_lock(fs);
        ret = fs->op->read(buf, len, p);
        fs_unlock(fs);
    }
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

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->write != NULL) {
        fs_lock(fs);
        ret = fs->op->write(buf, len, p);
        fs_unlock(fs);
    }
    return ret;
}

int sys_lseek(int fd, int offset, int dir) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->seek != NULL) {
        fs_lock(fs);
        ret = fs->op->seek(p, offset, dir);
        fs_unlock(fs);
    }
    return ret;
}

int sys_close(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file error");
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;        
    }

    // 断言文件已经打开
    ASSERT(p->ref > 0);

    // 减少ref
    if(--p->ref == 0) { // 若为0, 那么关闭文件
        fs_t* fs = p->fs;
        if(fs->op->close != NULL) {
            fs_lock(fs);
            fs->op->close(p);
            fs_unlock(fs);
        }
        file_dec_ref(p);
    }
    // 释放fd
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

    int ret = -1;
    if(fs->op->stat != NULL) {
        fs_lock(fs);
        ret = fs->op->stat(p, st);
        fs_unlock(fs);
    }
    return ret;
}

int sys_ioctl(int fd, int cmd, int arg0, int arg1) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    int ret = -1;
    fs_t* fs = p->fs;
    if(fs->op->ioctl != NULL) {
        fs_lock(fs);
        ret = fs->op->ioctl(p, cmd, arg0, arg1);
        fs_unlock(fs);
    }
    return ret;
}

int sys_dup(int fd) {
    if(is_fd_bad(fd)) {
        log_print("file %d is invalid.", fd);
        return -1;
    }

    file_t* p = get_task_file(fd);
    if(p == NULL) {
        log_print("file not opened.");
        return -1;
    }

    // 新增new指向fd对应的文件
    int new = task_alloc_fd(p);
    if(new < 0) {
        log_print("no task file avaliable.");
        return -1;
    }
    
    // 增加文件的引用计数
    file_inc_ref(p);
    return new;
}

int sys_opendir(const char* path, DIR* dir) {
    int ret = -1;
    if(root_fs->op->opendir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->opendir(root_fs, path, dir);
        fs_unlock(root_fs);
    }
    return ret;
}

int sys_readdir(DIR* dir, struct dirent* dirent) {
    int ret = -1;
    if(root_fs->op->readdir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->readdir(root_fs, dir, dirent);
        fs_unlock(root_fs);
    }
    return ret;
}

int sys_closedir(DIR* dir) {
    int ret = -1;
    if(root_fs->op->closedir != NULL) {
        fs_lock(root_fs);
        ret = root_fs->op->closedir(root_fs, dir);
        fs_unlock(root_fs);
    }
    return ret;
}


static void mount_list_init() {
    // 初始化空闲与挂载链表
    list_init(&mounted_list);
    list_init(&free_list);

    // 将系统文件表组织成链表
    for(int i = 0; i < FS_TABLE_SIZE; i++) {
        list_insert_first(&free_list, &fs_table[i].node);
    }
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

    /**
     * 主要流程:
     *      1. 判断当前挂载点没有被挂载过
     *      2. 分配一个空闲的文件系统表
     *      3. 初始化文件系统
     *      4. 执行挂载
     */

    fs_t* fs = NULL;
    log_print("mount file system, name: %s, dev: %x,", mount_point, dev_major);

    // 1. 遍历挂载链表, 判断当前文件是否挂载过
    list_node_t* curr = list_first(&mounted_list);
    while(curr != NULL) {
        // 若fs的挂载点与入参的挂载点一样就退出
        fs_t* p = list_entry_of(curr, fs_t, node);
        if(kernel_strncmp(fs->mount_point, mount_point, FS_MOUNTP_SIZE) == 0) {
            log_print("fs already mounted.");
            goto mount_failed;
        }
        curr = list_node_next(curr);
    }

    // 2. 从空闲链表中分配新的fs
    list_node_t* free_node = list_remove_first(&free_list);
    if(free_node == NULL) {
        log_print("no free fs, mount failed.");
        goto mount_failed;
    }
    fs = list_entry_of(free_node, fs_t, node);

    // 3. 初始化fs
    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strncpy(fs->mount_point, mount_point, FS_MOUNTP_SIZE);
    
    fs_op_t* op = get_fs_op(type, dev_major);   // 获取当前fs的操作函数
    if(op == NULL || op->mount == NULL) {
        log_print("unsupport fs type: %d or no mount function.", type);
        goto mount_failed;
    }
    fs->op = op;
    fs->mtx = NULL;

    // 4. 执行挂载, 调用fs各自的挂载函数
    if(op->mount(fs, dev_major, dev_minor) < 0) {
        log_print("mount fs %s failed.", mount_point);
        goto mount_failed;
    }
    list_insert_last(&mounted_list, &fs->node);     // 将节点移至挂载链表中

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

int string_to_int(const char* path, int* num) {
    int x = 0;
    const char* p = path;
    while(*p) {
        if(*p < '0' || *p > '9') {
            return -1;
        }
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