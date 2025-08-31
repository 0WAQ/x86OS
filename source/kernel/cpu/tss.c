#include "cpu/tss.h"
#include "core/task/task_t.h"
#include "cpu/cpu.h"
#include "core/memory/pmem.h"
#include "core/memory/vmem.h"
#include "common/cfg.h"
#include "tools/klib.h"

extern task_manager_t task_manager;

int tss_init(tss_t* tss, u32_t flag, u32_t entry, u32_t esp) {
    
// 1.创建tss, 注册到gdt中

    // 分配一个gdt表项
    int tss_sel = gdt_alloc_desc_with_guard();
    if(tss_sel < 0) {
        goto tss_init_failed;
    }

    // 注册为tss段，段的起始地址就是&task->tss，偏移为tss的大小
    set_segment_desc(tss_sel, (u32_t)tss, sizeof(tss_t), 
        DESC_ATTR_P | DESC_ATTR_DPL0 | DESC_ATTR_TYPE_TSS   
    );

// 2.初始化tss段
    kmemset(tss, 0, sizeof(tss_t));

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
    tss->cs = code_sel;
    tss->ss = tss->ds = tss->es = tss->fs = tss->gs = data_sel;


    tss->ss0 = KERNEL_SELECTOR_DS;     // 任务的内核栈段

    // 分配一页内存作为内核栈
    u32_t kernel_stack = pmem_alloc_page();
    if(kernel_stack == 0) {
        goto tss_init_failed;
    }

    // 若未指定用户栈那么就用内核栈, 一般是特权级为0的进程, 如idle
    tss->esp = esp ? esp : kernel_stack + MEM_PAGE_SIZE;
    tss->esp0 = kernel_stack + MEM_PAGE_SIZE;

    tss->eip = entry;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    
    tss->iomap = 0;

    // 创建用户自己的页目录表
    u32_t page_dir = vmem_create_uvms();
    if(page_dir == 0) {
        goto tss_init_failed;
    }
    tss->cr3 = page_dir;

    return tss_sel;

tss_init_failed:
    gdt_free_desc_with_guard(tss_sel);
    if(kernel_stack) {
        vmem_free_page(kernel_stack);
    }
    if(page_dir) {
        gdt_free_desc_with_guard(tss_sel);
    }
    return -1;
}