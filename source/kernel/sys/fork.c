#include "sys/syscall.h"

int sys_fork() {

    task_t* parent = get_curr_task();

    // 分配任务结构
    task_t* child = alloc_task();
    if(child == NULL) {
        goto sys_fork_failed;
    }

    // 获取父进程的栈0的esp寄存器(只有在切换走时才被更新), 强转为syscall_frame_t*
    exception_frame_t* frame = (exception_frame_t*)(parent->tss.esp0 - sizeof(exception_frame_t));

    // 初始化子进程
    int ret = task_init(child, parent->name, 0, frame->eip,
                        frame->esp3);
    
    if(ret < 0) {
        goto sys_fork_failed;
    }

    // 拷贝父进程的文件描述符表
    copy_opened_files(child);

    // 从父进程的栈中获取状态, 写入tss
    tss_t* tss = &child->tss;
    tss->eax = 0;   // 子进程返回0
    tss->ebx = frame->ebx;
    tss->edx = frame->edx;
    tss->ecx = frame->ecx;
    tss->esi = frame->esi;
    tss->edi = frame->edi;
    tss->ebp = frame->ebp;
    tss->cs = frame->cs;
    tss->ds = frame->ds;
    tss->es = frame->es;
    tss->fs = frame->fs;
    tss->gs = frame->gs;
    tss->eflags = frame->eflags;

    child->parent = parent;
    // TODO: 不应该是复制, 而是共享页表
    // 将父进程的页表映射复制到子进程
    if((tss->cr3 = memory_copy_uvm(parent->tss.cr3, child->tss.cr3)) == 0) {
        goto sys_fork_failed;
    }

    // 将子进程加入到就绪队列
    task_start(child);

    // 创建成功, 返回子进程的pid
    return child->pid;

sys_fork_failed:
    if(child) {
        task_uninit(child);
        free_task(child);
    }
    return -1;
}