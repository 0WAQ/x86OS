/**
 * 
 * 任务管理
 * 
 */
#include "core/task.h"
#include "core/memory.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "common/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"

// 任务管理器
static task_manager_t task_manager;

// 空闲任务的栈
static uint32_t idle_task_stack[IDLE_STACK_SIZE];

int task_init(task_t* task, const char* name, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t*)0);
    tss_init(task, entry, esp);

    // 初始化任务时，执行以下操作的原因:
    // 因为从其它任务首次切换到该任务时，会先恢复该任务的状态
    //  但该任务是首次运行，之前没有状态(并且该任务的调用栈为空)
    // uint32_t* pesp = (uint32_t*)esp;
    // if(pesp) {
    //     *(--pesp) = entry;  // eip, 即返回参数
    //     *(--pesp) = 0;      // ebp
    //     *(--pesp) = 0;      // ebx
    //     *(--pesp) = 0;      // esi
    //     *(--pesp) = 0;      // edi
    //     task->stack = pesp; // 设置该任务的栈顶
    // }

    // 设置任务名和状态
    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_ticks;
    task->sleep_ticks = 0;

    list_node_init(&task->all_node);
    list_node_init(&task->wait_node);
    list_node_init(&task->run_node);

    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection();


    // 插入到任务队列中
    list_insert_last(&task_manager.task_list, &task->all_node);
    
    // 插入到就绪队列中(设置任务为就绪态)
    set_task_ready(task);


    irq_leave_protectoin(state);
    /////////////////////////////////////////// 退出临界区

    return 0;
}

int tss_init(task_t* task, uint32_t entry, uint32_t esp) {
    
// 将tss注册到gdt中

    // 寻找第一个空闲的gdt表项
    int tss_sel = gdt_alloc_desc();
    if(tss_sel < 0) {
        log_print("alloc tss failed.");
        return -1;
    }

    // 注册为tss段，段的起始地址就是&task->tss，偏移为tss的大小
    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t), 
        DESC_ATTR_P | DESC_ATTR_DPL0 | DESC_ATTR_TYPE_TSS   
    );

    // 将段选择子保存起来
    task->tss_sel = tss_sel;

// 初始化tss段

    kernel_memset(&task->tss, 0, sizeof(tss_t));
    
    // 使用数据段寄存器访问段时要算上RPL
    int code_sel = task_manager.code_sel | DESC_ATTR_RPL3;
    int data_sel = task_manager.data_sel | DESC_ATTR_RPL3;

    task->tss.cs = code_sel;
    task->tss.ss = task->tss.ds = task->tss.es = task->tss.fs = task->tss.gs = data_sel;
    
    task->tss.ss = data_sel;
    task->tss.ss0 = KERNEL_SELECTOR_DS;

    task->tss.esp = task->tss.esp0 = esp;
    
    task->tss.eip = entry;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    
    task->tss.iomap = 0;

    // 创建用户自己的页目录表
    uint32_t page_dir = memory_create_user_vm();
    if(page_dir == 0) {
        gdt_free_desc(tss_sel);
        return -1;
    }
    task->tss.cr3 = page_dir;

    return 0;
}

void task_switch_from_to(task_t* from, task_t* to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}


void task_manager_init() {

    // 创建两个特权级为3的段
    int data_sel = gdt_alloc_desc();
    segment_desc_set(data_sel, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_S_USR | 
        DESC_ATTR_TYPE_DATA | DESC_ATTR_TYPE_RW | DESC_ATTR_D
    );
    task_manager.data_sel = data_sel;

    int code_sel = gdt_alloc_desc();
    segment_desc_set(code_sel, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_S_USR | 
        DESC_ATTR_TYPE_CODE | DESC_ATTR_TYPE_RW | DESC_ATTR_D
    );
    task_manager.code_sel = code_sel;

    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    // 初始化空闲任务
    idle_task_init();

    task_manager.curr_task = (task_t*)0;
}

void first_task_init() {

    // 在first_task_entry.S中定义
    extern void first_task_entry();

    // 在链接脚本kernel.lds中定义
    extern uint8_t s_first_task[], e_first_task[];
    extern uint8_t* sssssss;

    // first_task需要使用的内存, 只包含 .text .data .bss .rodata
    uint32_t copy_size = (uint32_t)(e_first_task - s_first_task);
    uint32_t alloc_size = 10 * MEM_PAGE_SIZE;   // 还有栈段需要计算, 这里不计算了
    ASSERT(copy_size < alloc_size);

    // 这里不需要给参数，因为当cpu从当前任务切换走时，会自动将状态保存，只要保证有地方可去就行
    // cpu会自动保存来源
    task_init(&task_manager.first_task, "first task", (uint32_t)first_task_entry, 0);
    task_manager.curr_task = &task_manager.first_task;

    // 加载tss
    ltr(task_manager.first_task.tss_sel);

    // 加载cr3
    mmu_set_page_dir(task_manager.first_task.tss.cr3);
    
    // 为first_task分配物理页, 从first_task_entry(0x80000000)处分配
    memory_alloc_page_for((uint32_t)first_task_entry, alloc_size, PTE_P | PTE_W);

    // 将first_task从内核移指用户
    kernel_memcpy(first_task_entry, s_first_task, copy_size);
}

task_t* get_first_task() {
    return &task_manager.first_task;
}

void idle_task_init() {
    task_init(&task_manager.idle_task, "idle task",
        (uint32_t)idle_task_entry, (uint32_t)(idle_task_stack + IDLE_STACK_SIZE));
}

task_t* get_idle_task() {
    return &task_manager.idle_task;
}

void idle_task_entry() {
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

void set_task_sleep(task_t* task, uint32_t ticks) {
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

int sys_yield() {

    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection();


    // 判断就绪队列中是否有任务
    if(list_count(&task_manager.ready_list) > 1) {
        task_t* curr_task = get_curr_task();
        
        // 若就绪队列中还有其它任务,则将当前任务移入队列尾部
        set_task_block(curr_task);  // 先删除
        set_task_ready(curr_task);  // 再尾插
    
        // 任务切换
        task_dispatch();
    }


    irq_leave_protectoin(state);
    /////////////////////////////////////////// 退出临界区

    return 0;
}

task_t* get_next_task() {

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

    irq_leave_protectoin(state);
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

void sys_sleep(uint32_t ms) {
    
    /////////////////////////////////////////// 进入临界区
    irq_state_t state = irq_enter_protection(); 
    
    task_t* curr = get_curr_task();
    set_task_block(curr);
    set_task_sleep(curr, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);

    task_dispatch();

    irq_leave_protectoin(state);
    /////////////////////////////////////////// 退出临界区

}