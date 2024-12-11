/**
 * 
 * 文件系统C文件
 * 
 */

#include "fs/fs.h"
#include "fs/file.h"
#include "dev/dev.h"
#include "dev/console.h"
#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "tools/list.h"
#include "common/cpu_instr.h"
#include <sys/stat.h>
#include <sys/file.h>

static list_t mounted_list;             // 挂载链表
static list_t free_list;                // 空闲链表
static fs_t fs_table[FS_TABLE_SIZE];    // fs表
extern fs_op_t devfs_op;                // TODO: 临时的

void fs_init() {

    // 初始化文件系统的链表
    mount_list_init();

    // 初始化文件表
    file_table_init();

    // 挂载devfs
    fs_t* fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != NULL);
}

// TODO: 临时使用
void read_disk(uint32_t sector, uint32_t sector_cnt, uint8_t* buffer) {
    
    // 读取LBA参数
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_cnt >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_cnt));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

    // 读取数据
    uint16_t* buf = (uint16_t*)buffer;
    while(sector_cnt-- > 0) {

        // 读前检查，等待数据就位
        while((inb(0x1F7) & 0x88) != 0x08);

        // 读取数据并将其写入到缓存中
        for(int i = 0; i < 512 / 2; i++) {
            *buf++ = inw(0x1F0);
        }
    }
}

// TODO: TEMP
static int is_path_valid(const char* path) {
    if((path == NULL) || (path[0] == '\0')) {
        return 0;
    }
    return 1;
}

int sys_open(const char* filename, int flags, ...) {

    // TODO: 暂且用于execve以打开shell
    if(kernel_strncmp(filename, "/shell.elf", 10) == 0) {
        read_disk(5000, 80, (uint8_t*)TEMP_ADDR);
        temp_pos = (uint8_t*)TEMP_ADDR;
        return TEMP_FILE_ID;
    }

    file_t* file = file_alloc();
    if(file == NULL) {
        goto sys_open_failed;
    }

    int fd = task_alloc_fd(file);
    if(fd < 0) {
        goto sys_open_failed;
    }

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
        // TODO: 缺省值
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
    // TODO: 
    if(fd == TEMP_FILE_ID) {
        kernel_memcpy(buf, temp_pos, len);
        temp_pos += len;
        return len;
    }

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
    if(fd == TEMP_FILE_ID) {
        temp_pos = (uint8_t*)(TEMP_ADDR + offset);
        return 0;
    }

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
    if(fd == TEMP_FILE_ID) {
        return 0;
    }

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
        return &(devfs_op);
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

    // 取出空闲链表的一个节点, 将其分配给fs
    list_node_t* free_node = list_remove_first(&free_list);
    if(free_node == NULL) {
        log_print("no free fs, mount failed.");
        goto mount_failed;
    }
    fs = list_entry_of(free_node, fs_t, node);

    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strncpy(fs->mount_point, mount_point, FS_MOUNTP_SIZE);
    // fs->node = free_node;

    // 获取当前fs的操作函数
    fs_op_t* op = get_fs_op(type, dev_major);
    if(op == NULL) {
        log_print("unsupport fs type: %d", type);
        goto mount_failed;
    }
    fs->op = op;

    // 执行挂载
    if(op->mount(fs, dev_major, dev_minor) < 0) {
        log_print("mount fs %s failed.", mount_point);
        goto mount_failed;
    }

    // 将节点移至挂载链表中
    list_insert_last(&mounted_list, &fs->node);
    return fs;

mount_failed:
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