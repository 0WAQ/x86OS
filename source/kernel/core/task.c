/**
 * 
 * 任务管理
 * 
 */
#include "core/task.h"
#include "core/memory.h"
#include "sys/syscall.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "fs/fs.h"
#include "fs/file.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"
#include "os_cfg.h"

// 任务管理器
task_manager_t task_manager;

// 任务控制块
task_t task_table[TASK_NR];
mutex_t task_table_mutex;

int task_init(task_t* task, const char* name, u32_t flag, u32_t entry, u32_t esp) {
    ASSERT(task != NULL);

    // 初始化tss
    tss_init(task, flag, entry, esp);

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
    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->heap_start = 0;
    task->heap_end = 0;
    task->parent = task_manager.curr_task;
    task->state = TASK_CREATED;
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_ticks;
    task->sleep_ticks = 0;
    task->status = 0;

    list_node_init(&task->all_node);
    list_node_init(&task->wait_node);
    list_node_init(&task->run_node);

    // 初始化文件表
    kernel_memset((void*)&task->file_table, 0, sizeof(task->file_table));

    return 0;
}

void task_start(task_t* task) {

    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection();


    // 插入到任务队列中
    list_insert_last(&task_manager.task_list, &task->all_node);
    
    // 插入到就绪队列中(设置任务为就绪态)
    set_task_ready(task);


    irq_leave_protection(state);
    /////////////////////////////////////////// 退出临界区

}

int task_uninit(task_t* task) {
    if(task->tss_sel) {
        gdt_free_desc(task->tss_sel);
    }

    if(task->tss.esp0) {
        memory_free_page(task->tss.esp - MEM_PAGE_SIZE);
    }

    if(task->tss.cr3) {
        memory_destory_uvm(task->tss.cr3);
    }

    kernel_memset(task, 0, sizeof(task_t));
}

static int tss_init(task_t* task, u32_t flag, u32_t entry, u32_t esp) {
    
// 1.创建tss, 注册到gdt中

    // 分配一个gdt表项
    int tss_sel = gdt_alloc_desc();
    if(tss_sel < 0) {
        goto tss_init_failed;
    }

    // 注册为tss段，段的起始地址就是&task->tss，偏移为tss的大小
    set_segment_desc(tss_sel, (u32_t)&task->tss, sizeof(tss_t), 
        DESC_ATTR_P | DESC_ATTR_DPL0 | DESC_ATTR_TYPE_TSS   
    );

    // 将段选择子保存起来
    task->tss_sel = tss_sel;


// 2.初始化tss段

    kernel_memset(&task->tss, 0, sizeof(tss_t));
    
    int code_sel, data_sel; 
    if(flag & TASK_FLAGS_SYSTEM) {
        code_sel = KERNEL_SELECTOR_CS;
        data_sel = KERNEL_SELECTOR_DS;    
    }
    else {
        // 使用数据段寄存器访问段时要算上RPL
        code_sel = task_manager.code_sel | DESC_ATTR_RPL3;
        data_sel = task_manager.data_sel | DESC_ATTR_RPL3;
    }
    task->tss.cs = code_sel;
    task->tss.ss = task->tss.ds = task->tss.es = task->tss.fs = task->tss.gs = data_sel;


    task->tss.ss0 = KERNEL_SELECTOR_DS;     // 任务的内核栈段

    // 分配一页内存作为内核栈
    u32_t kernel_stack = memory_alloc_page();
    if(kernel_stack == 0) {
        goto tss_init_failed;
    }

    // 若未指定用户栈那么就用内核栈, 一般是特权级为0的进程, 如idle
    task->tss.esp = esp ? esp : kernel_stack + MEM_PAGE_SIZE;
    task->tss.esp0 = kernel_stack + MEM_PAGE_SIZE;

    task->tss.eip = entry;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    
    task->tss.iomap = 0;

    // 创建用户自己的页目录表
    u32_t page_dir = memory_create_uvm();
    if(page_dir == 0) {
        goto tss_init_failed;
    }
    task->tss.cr3 = page_dir;

    return 0;

tss_init_failed:
    gdt_free_desc(tss_sel);
    if(kernel_stack) {
        memory_free_page(kernel_stack);
    }
    if(page_dir) {
        gdt_free_desc(tss_sel);
    }
    return -1;
}

static void task_switch_from_to(task_t* from, task_t* to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}


void task_manager_init() {

    kernel_memset(task_table, 0, sizeof(task_table));
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

    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    // 初始化空闲任务
    u32_t flag = TASK_FLAGS_SYSTEM;
    task_manager.idle_task = alloc_task();
    task_init(task_manager.idle_task, "idle task", flag, (u32_t)idle_task_entry, 0);

    task_manager.curr_task = NULL;

    task_start(task_manager.idle_task);
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
    task_manager.first_task = alloc_task();
    task_init(task_manager.first_task, "first task", flag, 
            (u32_t)first_task_entry, (u32_t)first_task_entry + alloc_size);
    
    task_manager.first_task->heap_start = (u32_t)e_first_task;
    task_manager.first_task->heap_end = (u32_t)e_first_task;

    task_manager.curr_task = task_manager.first_task;

    // 加载tss到tr寄存器中
    ltr(task_manager.first_task->tss_sel);

    // 加载cr3
    mmu_set_page_dir(task_manager.first_task->tss.cr3);
    
    // 为first_task分配物理页, 从first_task_entry(0x80000000)处分配
    memory_alloc_page_for((u32_t)first_task_entry, alloc_size, PTE_P | PTE_W | PTE_U);

    // 将first_task从内核移指用户
    kernel_memcpy((void*)first_task_entry, (void*)&s_first_task, copy_size);

    // 启动一号进程
    task_start(task_manager.first_task);
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

void set_task_ready(task_t* task) {

    if(task == get_idle_task()) {
        return;
    }

    // 执行插入
    list_insert_last(&task_manager.ready_list, &task->run_node);

    // 设置任务状态
    task->state = TASK_READY;
}

void set_task_block(task_t* task) {
    
    if(task == get_idle_task()) {
        return;
    }

    // 执行删除
    list_remove(&task_manager.ready_list, &task->run_node);

    // 不设置状态，因为不确定
}

void set_task_sleep(task_t* task, u32_t ticks) {
    if(ticks == 0) {
        return;
    }

    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;

    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

void set_task_wakeup(task_t* task) {
    list_remove(&task_manager.sleep_list, &task->run_node);
}

task_t* get_curr_task() {
    return task_manager.curr_task;
}

static task_t* get_next_task() {

    if(list_count(&task_manager.ready_list) == 0) {
        return get_idle_task();
    }

    list_node_t* task_node = list_first(&task_manager.ready_list);
    return list_entry_of(task_node, task_t, run_node);
}

void task_dispatch() {
    
    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection();


    // 获取当前任务和下个任务
    task_t* from = get_curr_task();
    task_t* to   = get_next_task();

    // 若与当前任务不相同, 那么就切换过去
    if(to != from) {
        task_manager.curr_task = to;
        to->state = TASK_RUNNING;
        task_switch_from_to(from, to);
    }

    irq_leave_protection(state);
    /////////////////////////////////////////// 退出临界区
}

void task_time_tick() {
    task_t* curr_task = get_curr_task();

    // 处理时间片
    if(--curr_task->slice_ticks == 0) {
        
        // 时间片用完, 重新设置时间片
        curr_task->slice_ticks = curr_task->time_ticks;

        // 若就绪队列中还有其它任务,则将当前任务移入队列尾部
        set_task_block(curr_task);  // 先删除
        set_task_ready(curr_task);  // 再尾插
    
        task_dispatch();
    }

    list_node_t* curr = list_first(&task_manager.sleep_list);
    while(curr) {
        list_node_t* next = list_node_next(curr);

        task_t* task = list_entry_of(curr, task_t, run_node);
        // 睡眠时间已到
        if(--task->sleep_ticks == 0) {
            set_task_wakeup(task);
            set_task_ready(task);
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

task_t* alloc_task() {
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

void free_task(task_t* task) {
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
       elf_hdr.e_machine != ET_386 || 
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
    int ret = _memory_alloc_page_for(page_dir, phdr->p_vaddr, phdr->p_memsz, PTE_P | PTE_U | PTE_W);
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
        u32_t paddr = memory_vaddr_to_paddr(page_dir, vaddr);
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

int copy_args(char* to, u32_t page_dir, int argc, char** argv) {

    // 再stack_top中一次写入argc, argv
    task_args_t task_args;
    task_args.argc = argc;
    task_args.argv = (char**)(to + sizeof(task_args_t));

    char* dest_arg = to + sizeof(task_args_t) + (argc + 1) * sizeof(char*);
    
    // argv表
    char** dest_arg_tb = (char**)memory_vaddr_to_paddr(page_dir, (u32_t)(to + sizeof(task_args_t)));
    ASSERT(dest_arg_tb != 0);

    for(int i = 0; i < argc; i++) {
        char* from = argv[i];

        // 不能用strcpy, 因为to和argv不在一个页表里
        int len = kernel_strlen(from) + 1;      // 包含结束符
        int ret = memory_copy_uvm_data((u32_t)dest_arg, page_dir, (u32_t)from, len);
        ASSERT(ret >= 0);
        
        dest_arg_tb[i] = dest_arg;
        dest_arg += len;
    }

    if(argc) {
        dest_arg_tb[argc] = '\0';
    }

    // 写入task_args
    return memory_copy_uvm_data((u32_t)to, page_dir, (u32_t)&task_args, sizeof(task_args_t));
}

file_t* get_task_file(int fd) {
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
