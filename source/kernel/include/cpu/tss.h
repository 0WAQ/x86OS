#ifndef TSS_H
#define TSS_H

#include "cpu/tss_t.h"

/**
 * @brief 初始化任务状态段
 * @param esp 任务的用户栈
 * @return 返回 tss 的段选择子
 */
int tss_init(tss_t* tss, u32_t flag, u32_t entry, u32_t esp);


#ifndef TSS_T_H
#endif // TSS_T_H

#endif // TSS_H
