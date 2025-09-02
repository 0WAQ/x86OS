/**
 * 任务管理模块
 */
#include "task/task.h"
#include "common/types.h"
#include "memory/vmem.h"
#include "sys/syscall.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "cpu/mmu.h"
#include "cpu/tss.h"
#include "fs/file.h"
#include "task/cfs.h"
#include "tools/list.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/asm.h"
#include "common/cfg.h"

// 任务管理器
task_manager_t task_manager;

// 任务控制块
task_t task_table[TASK_NR];
mutex_t task_table_mutex;

extern const u32_t sched_nice_to_weight[40];

int task_init(task_t* task, cfs_rq_t* rq, int prio,
        const char* name, u32_t flag, u32_t entry, u32_t esp)
{
    ASSERT(task != NULL);

    // 初始化tss
    task->tss_sel = tss_init(&task->tss, flag, entry, esp);
    if (task->tss_sel < 0) {
        return -1;
    }

    // 初始化任务时，执行以下操作的原因:
    // 因为从其它任务首次切换到该任务时，会先恢复该任务的状态
    //  但该任务是首次运行，之前没有状态(并且该任务的调用栈为空)
    // u32_t* pesp = (u32_t*)esp;
    // if(pesp) {
    //     *(--pesp) = entry;  // eip, 即返回参数
    //     *(--pesp) = 0;      // ebp
    //     *(--pesp) = 0;      // ebx
    //     *(--pesp) = 0;      // esi
    //     *(--pesp) = 0;      // edi
    //     task->stack = pesp; // 设置该任务的栈顶
    // }

    // 设置任务的各种属性
    kstrncpy(task->name, name, TASK_NAME_SIZE);
    task->heap_start = 0;
    task->heap_end = 0;
    task->parent = task_manager.curr_task;
    task->state = TASK_CREATED;
    task->sleep_ticks = 0;
    task->status = 0;
    task->prio = prio;
    
    sched_entity_t* se = &task->sched_entity;
    se->on_rq = false;
    se->load.weight = sched_nice_to_weight[prio + 20];
    // TODO: 实际不应该这样, 因为若有很多任务不断加入, 那么其余任务会被一直抢占    
    se->vruntime = rq->min_vruntime;

    rb_node_init(&se->cfs_node);

    list_node_init(&task->all_node);
    list_node_init(&task->sleep_node);
    list_node_init(&task->wait_node);

    // 初始化文件表
    kmemset((void*)&task->file_table, 0, sizeof(task->file_table));
    return 0;
}

void task_start(task_t* task) {
    irq_state_t state = irq_enter_protection();

    // 插入到任务队列中
    list_insert_last(&task_manager.task_list, &task->all_node);

    // 插入到就绪队列中(设置任务为就绪态)
    task_set_ready(task);

    irq_leave_protection(state);
}

void task_uninit(task_t* task) {
    if(task->tss_sel) {
        gdt_free_desc_with_guard(task->tss_sel);
    }
    if(task->tss.esp0) {
        vmem_free_page(task->tss.esp - MEM_PAGE_SIZE);
    }
    if(task->tss.cr3) {
        vmem_destory_uvms(task->tss.cr3);
    }
    kmemset(task, 0, sizeof(task_t));
}

static void task_switch(task_t* from, task_t* to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}


void task_manager_init() {
    kmemset(task_table, 0, sizeof(task_table));
    mutex_init(&task_table_mutex);

    // 创建两个特权级为3的段
    int data_sel = gdt_alloc_desc();
    set_segment_desc(data_sel, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_S_USR | 
        DESC_ATTR_TYPE_DATA | DESC_ATTR_TYPE_RW | DESC_ATTR_D
    );
    task_manager.data_sel = data_sel;

    int code_sel = gdt_alloc_desc();
    set_segment_desc(code_sel, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_S_USR | 
        DESC_ATTR_TYPE_CODE | DESC_ATTR_TYPE_RW | DESC_ATTR_D
    );
    task_manager.code_sel = code_sel;

    cfs_init(&task_manager.cfs_rq);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    // 初始化空闲任务
    u32_t flag = TASK_FLAGS_SYSTEM;
    task_manager.idle_task = task_alloc();
    // TODO: 不应该将 idle task 放入到 调度队列中, 只有 cfs 中没有任务才运行它, 所以其压根不是
    task_init(task_manager.idle_task, &task_manager.cfs_rq, 0,
        "idle task", flag, (u32_t)idle_task_entry, 0);

    task_manager.curr_task = NULL;

    list_insert_last(&task_manager.task_list, &task_manager.idle_task->all_node);
    task_manager.idle_task->state = TASK_READY;
}

void first_task_init() {

    // 在first_task_entry.S中定义
    extern void first_task_entry();

    // 在链接脚本kernel.lds中定义
    extern u8_t s_first_task[], e_first_task[];

    // 将一号进程移指用户态
    // first_task需要使用的内存, 只包含 .text .data .bss .rodata
    u32_t copy_size = (u32_t)(e_first_task - s_first_task);   // 需要移动的字节
    u32_t alloc_size = 10 * MEM_PAGE_SIZE;   // 还有栈段需要计算, 这里不计算了
    ASSERT(copy_size < alloc_size);


    // 这里不需要给参数，因为当cpu从当前任务切换走时，会自动将状态保存，只要保证有地方可去就行
    // cpu会自动保存来源

    u32_t flag = TASK_FLAGS_USER;
    task_manager.first_task = task_alloc();
    // TEST:
    task_init(task_manager.first_task, &task_manager.cfs_rq, 0,
        "first task", flag, (u32_t)first_task_entry, (u32_t)first_task_entry + alloc_size);
    
    task_manager.first_task->heap_start = (u32_t)e_first_task;
    task_manager.first_task->heap_end = (u32_t)e_first_task;

    task_manager.curr_task = task_manager.first_task;

    // 加载tss到tr寄存器中
    ltr(task_manager.first_task->tss_sel);

    // 加载cr3
    mmu_set_page_dir(task_manager.first_task->tss.cr3);
    
    // 为first_task分配物理页, 从first_task_entry(0x80000000)处分配
    vmem_alloc_page((u32_t)get_curr_page_dir(), (u32_t)first_task_entry, alloc_size, PTE_P | PTE_W | PTE_U);

    // 将first_task从内核移指用户
    kmemcpy((void*)first_task_entry, (void*)&s_first_task, copy_size);

    list_insert_last(&task_manager.task_list, &task_manager.first_task->all_node);
    task_manager.first_task->state = TASK_READY;
}

static task_t* get_first_task() {
    return task_manager.first_task;
}

static task_t* get_idle_task() {
    return task_manager.idle_task;
}

static void idle_task_entry() {
    for(;;) { hlt(); }
}

void task_set_ready(task_t* task) {
    if (task != get_idle_task()) {
        cfs_enqueue_task(&task_manager.cfs_rq, task);
        task->state = TASK_READY;
    }
}

void task_set_sleep(task_t* task, u32_t ticks) {
    if (ticks != 0) {
        task->sleep_ticks = ticks;
        list_insert_last(&task_manager.sleep_list, &task->sleep_node);
        task->state = TASK_SLEEPING;
    }
}

void task_set_wakeup(task_t* task) {
    list_remove(&task_manager.sleep_list, &task->sleep_node);
}

inline task_t* get_curr_task() {
    return task_manager.curr_task;
}

static task_t* get_next_task() {
    if (cfs_rq_empty(&task_manager.cfs_rq)) {
        return get_idle_task();
    }
    return cfs_pick_next_task(&task_manager.cfs_rq);
}

void task_dispatch() {
    irq_state_t state = irq_enter_protection();

    // 获取当前任务和下个任务
    task_t* from = get_curr_task();
    task_t* to   = get_next_task();

    if (to != get_idle_task()) {
        // cfs_dequeue_task(&task_manager.cfs_rq);
        cfs_erase_task(&task_manager.cfs_rq, to);
    }

    if (from != get_idle_task()) {
        u32_t delta_exec = 100; // TEST: 如何获取实际执行时间?
        cfs_update_vruntime(&from->sched_entity, delta_exec);

        if (from->state == TASK_READY) {
            cfs_enqueue_task(&task_manager.cfs_rq, from);
        }
    }

    // 若与当前任务不相同, 那么就切换过去
    if(to != from) {
        task_manager.curr_task = to;
        task_switch(from, to);
    }

    irq_leave_protection(state);
}

void task_time_tick() {
    // 处理睡眠队列中的任务
    list_node_t* curr = list_first(&task_manager.sleep_list);
    while(curr) {
        list_node_t* next = list_node_next(curr);
        task_t* task = entry_of(curr, task_t, sleep_node);

        // 睡眠时间已到
        if(--task->sleep_ticks == 0) {
            task_set_wakeup(task);
            task_set_ready(task);
        }
    
        curr = next;
    }

    task_dispatch();
}

void copy_opened_files(task_t* child) {
    task_t* parent = get_curr_task();
    for(int i = 0; i < TASK_OFILE_NR; i++) {
        file_t* file = parent->file_table[i];
        if(file) {
            file_inc_ref(file);
            child->file_table[i] = file;
        }
    }
}

task_t* task_alloc() {
    task_t* task = NULL;
    
    mutex_lock(&task_table_mutex);
    for(int i = 0; i < TASK_NR; i++) {
        task_t* curr = task_table + i;
        if(curr->name[0] == '\0') {
            task = curr;
            task->pid = i;
            break;
        }
    }
    mutex_unlock(&task_table_mutex);
    return task;
}

void task_free(task_t* task) {
    mutex_lock(&task_table_mutex);
    task->name[0] = '\0';
    mutex_unlock(&task_table_mutex);
}


u32_t load_elf_file(task_t* task, const char* filename, u32_t page_dir) {
    Elf32_Ehdr elf_hdr;
    Elf32_Phdr elf_phdr;

    // 以只读的方式打开
    int fd = sys_open(filename, 0);   // TODO: flags
    if(fd < 0) {
        log_print("open %s failed.", filename);
        goto load_elf_file_failed;
    }

    // 先读取文件头
    int len = sys_read(fd, (char*)&elf_hdr, sizeof(Elf32_Ehdr));
    if(len < sizeof(Elf32_Ehdr)) {
        log_print("elf hdr too small. size=%d", len);
        goto load_elf_file_failed;
    }

    // 判断文件是否是合格的elf
    if(elf_hdr.e_ident[0] != ELF_MAGIC || 
       elf_hdr.e_ident[1] != 'E'  ||
       elf_hdr.e_ident[2] != 'L'  ||
       elf_hdr.e_ident[3] != 'F')
    {
        log_print("check elf ident failed.");
        goto load_elf_file_failed;
    }

    // 其它检查
    if(elf_hdr.e_type != ET_EXEC || 
       elf_hdr.e_machine != EM_386 || 
       elf_hdr.e_entry == 0)
    {
        log_print("check elf type or entry failed.");
        goto load_elf_file_failed;
    }

    // 必须有程序头
    if(elf_hdr.e_phentsize == 0 ||
       elf_hdr.e_phoff == 0)
    {
        log_print("none programe header.");
        goto load_elf_file_failed;
    }

    // 加载程序头, 将内容拷贝到相应位置
    u32_t e_phoff = elf_hdr.e_phoff;
    for(u32_t i = 0; i < elf_hdr.e_phnum; ++i, e_phoff += elf_hdr.e_phentsize) {
        if(sys_lseek(fd, e_phoff, 0) < 0) {
            log_print("read file failed.");
            goto load_elf_file_failed;
        }

        // 读取程序头后解析
        len = sys_read(fd, (char*)&elf_phdr, sizeof(Elf32_Phdr));
        if(len < sizeof(Elf32_Phdr)) {
            log_print("read file failed.");
            goto load_elf_file_failed;            
        }

        // 检查
        if(elf_phdr.p_type != PT_LOAD || elf_phdr.p_vaddr < MEMORY_TASK_BASE) {
            continue;           
        }

        // 加载当前程序头
        int ret = load_phdr(fd, &elf_phdr, page_dir);
        if(ret < 0) {
            log_print("load program failed.");
            goto load_elf_file_failed;;
        }

        // 设置堆的位置(在bss区后边, bss区是最后一个节)
        task->heap_start = elf_phdr.p_vaddr + elf_phdr.p_memsz;
        task->heap_end  = task->heap_start;
    }

    sys_close(fd);
    return elf_hdr.e_entry;

load_elf_file_failed:
    if(fd >= 0) {
        sys_close(fd);
    }
    return 0;
}

int load_phdr(int fd, Elf32_Phdr* phdr, u32_t page_dir) {

    // 生成的elf文件必须是页边界对齐的
    ASSERT((phdr->p_vaddr & (MEM_PAGE_SIZE - 1)) == 0);

    // 分配空间
    int ret = vmem_alloc_page(page_dir, phdr->p_vaddr, phdr->p_memsz, PTE_P | PTE_U | PTE_W);
    if(ret < 0) {
        log_print("mo memory");
        goto load_phdr_failed;
    }

    // 调整读写位置
    if(sys_lseek(fd, phdr->p_offset, 0) < 0) {
        log_print("read file failed.");
        goto load_phdr_failed;
    }

    u32_t vaddr = phdr->p_vaddr;
    u32_t size = phdr->p_filesz;
    while(size > 0) {

        // 每次只拷贝最多一页内存
        u32_t cur_size = (size > MEM_PAGE_SIZE) ? MEM_PAGE_SIZE : size;
        u32_t paddr = vmem_vaddr_to_paddr(page_dir, vaddr);
        if(sys_read(fd, (char*)paddr, cur_size) < cur_size) {
            log_print("copy file failed.");
            goto load_phdr_failed;
        }

        size -= cur_size;
        vaddr += cur_size;
    }

    return 0;

load_phdr_failed:
    return -1;
}

int task_copy_args(char* to, u32_t page_dir, u32_t argc, char** argv) {
    // 再stack_top中一次写入argc, argv
    task_args_t task_args;
    task_args.argc = argc;
    task_args.argv = (char**)(to + sizeof(task_args_t));

    char* dest_arg = to + sizeof(task_args_t) + (argc + 1) * sizeof(char*);
    
    // argv表
    char** dest_arg_tb = (char**)vmem_vaddr_to_paddr(page_dir, (u32_t)(to + sizeof(task_args_t)));
    ASSERT(dest_arg_tb != 0);

    for(int i = 0; i < argc; i++) {
        char* from = argv[i];

        // 不能用strcpy, 因为to和argv不在一个页表里
        int len = kstrlen(from) + 1;      // 包含结束符
        int ret = vmem_copy_corss_uvms((u32_t)dest_arg, page_dir, (u32_t)from, len);
        ASSERT(ret >= 0);
        
        dest_arg_tb[i] = dest_arg;
        dest_arg += len;
    }

    if(argc) {
        dest_arg_tb[argc] = "\0";
    }

    // 写入task_args
    return vmem_copy_corss_uvms((u32_t)to, page_dir, (u32_t)&task_args, sizeof(task_args_t));
}

file_t* task_get_file(int fd) {
    if((fd >= 0) && (fd < TASK_OFILE_NR)) {
        file_t* file = get_curr_task()->file_table[fd];
        return file;
    }
    return NULL;
}

int task_alloc_fd(file_t* pfile) {
    task_t* task = get_curr_task();
    // 遍历寻找空闲的文件表 
    for(int i = 0; i < TASK_OFILE_NR; i++) {
        file_t* p = task->file_table[i];
        if(p == NULL) {
            task->file_table[i] = pfile;
            return i;
        }
    }
    return -1;
}

void task_remove_fd(int fd) {
    if((fd >= 0) && (fd < TASK_OFILE_NR)) {
        get_curr_task()->file_table[fd] = NULL;
    }
}
