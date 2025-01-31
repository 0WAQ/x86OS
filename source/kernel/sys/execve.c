#include "sys/syscall.h"

int sys_execve(char* path, char** argv, char** env) {
    task_t* task = get_curr_task();

    // 修改任务名
    kernel_memcpy(task->name, get_filename_from_path(path), TASK_NAME_SIZE);

    // 创建新的页目录表, 防止在中途出现错误
    u32_t old_page_dir = task->tss.cr3;
    u32_t new_page_dir = memory_create_uvm();

    // 解析elf文件到内存中
    u32_t entry = load_elf_file(task, path, new_page_dir);   // TODO: 第二个参数
    if(!entry) {
        goto sys_execve_failed;
    }

    // TODO: 用户栈不是已经分配过了吗

    // 准备用户栈, 为环境变量与参数预留足够的空间
    u32_t stack_top = MEM_TASK_STACK_TOP - MEM_TASK_ARG_SIZE;
    int ret = _memory_alloc_page_for(new_page_dir, 
                                     MEM_TASK_STACK_TOP - MEM_TASK_STACK_SIZE, 
                                     MEM_TASK_STACK_SIZE, PTE_P | PTE_U | PTE_W);
    if(ret < 0) {
        goto sys_execve_failed;
    }

    // 复制参数到用户栈中
    int argc = strings_count(argv); 
    ret = copy_args((char*)stack_top, new_page_dir, argc, argv);
    if(ret < 0) {
        goto sys_execve_failed;
    }

    /// 加载完毕

    // 改变当前进程的执行流以替换进程
    // syscall_frame_t* frame = (syscall_frame_t*)(task->tss.esp0 - sizeof(syscall_frame_t));
    exception_frame_t* frame = (exception_frame_t*)(task->tss.esp0 - sizeof(exception_frame_t));
    frame->eip = entry;
    frame->eax = frame->ebx = frame->ecx = frame->edx = 0;
    frame->esi = frame->edi = frame->ebp = 0;
    frame->eflags = EFLAGS_IF | EFLAGS_DEFAULT;

    // 修改用户栈, 要加上调用门的参数压栈空间
    frame->esp3 = stack_top; // 通过中断进入内核时, 不会在用户栈压入其它参数

    // 切换至新的页表
    task->tss.cr3 = new_page_dir;
    mmu_set_page_dir(new_page_dir);

    // 销毁原页表
    memory_destory_uvm(old_page_dir);

    return 0;

sys_execve_failed:

    // 切换至旧页表, 销毁新页表
    if(new_page_dir) {
        task->tss.cr3 = old_page_dir;
        mmu_set_page_dir(old_page_dir);
        memory_destory_uvm(new_page_dir);
    }
    return -1;
}