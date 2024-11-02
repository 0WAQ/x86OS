/**
 *
 * task.c中函数的汇编实现
 *
 */
    .text
    .global simple_switch

# FIXME: 手动执行切换, 暂不使用
simple_switch:  # simple_switch(&from->stack, to->stack)
    mov 4(%esp), %eax   # 取&from->stack
    mov 8(%esp), %edx   # 取to->stack

    /**
     * 切换任务时，保存当前任务的状态
     *    1.无需保存：
     *       cs, ds, es, fs, gs(因为是平坦模型)
     *       eax, ecx, edx编译器会自动保存
     *       eip无需保存，已经在栈中
     *       esp需要手动设置为新任务的状态
     *    2.需要保存：
     *       esi, edi, ebp, ebx需要保存
     */

    # 保存当前任务的状态
    push %ebp
    push %ebx
    push %esi
    push %edi
    
    # 切换栈
    mov %esp, (%eax)    # from->stack = esp
    mov %edx, %esp      # esp = to->stack

    # 恢复新任务的状态
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret # 相当于恢复eip，执行完后，任务就正式切换了
