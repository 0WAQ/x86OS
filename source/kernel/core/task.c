/**
 * 
 * 任务管理
 * 
 */
#include "core/task.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "cpu/cpu.h"
#include "os_cfg.h"

int task_init(task_t* task, uint32_t entry, uint32_t esp) {
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
    
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;
    
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.eip = entry;
    
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    return 0;
}

void task_switch_from_to(task_t* from, task_t* to) {
    switch_to_tss(to->tss_sel);
    // simple_switch(&from->stack, to->stack);
}

