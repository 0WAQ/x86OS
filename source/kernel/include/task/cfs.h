#ifndef CFS_H
#define CFS_H

#include "common/types.h"
#include "task/cfs_t.h"
#include "task/task_t.h"
#include "tools/rbtree_t.h"

void cfs_init(cfs_rq_t* rq);

void cfs_enqueue_task(cfs_rq_t* rq, task_t* p);
task_t* cfs_dequeue_task(cfs_rq_t* rq);
void cfs_erase_task(cfs_rq_t* rq, task_t* p);

task_t* cfs_pick_next_task(cfs_rq_t* rq);

/**
 * @brief 根据权重和实际运行时间, 计算 delta_vruntime
 */
u32_t cfs_calc_delta_fair(u32_t delta_exec, u32_t load_weight);

/**
 * @brief 更新权重和 vruntime
 */
void cfs_update_vruntime(sched_entity_t* se, u32_t delta_exec);
void cfs_update_load_set(load_weight_t* lw, u32_t w);
void cfs_update_load_add(load_weight_t* lw, u32_t inc);
void cfs_update_load_sub(load_weight_t* lw, u32_t dec);

bool cfs_rq_empty(cfs_rq_t* rq);

/**
 * @brief 检验红黑树的正确性
 */
bool rb_check(rb_root* root, u32_t nr); // TODO:
int rb_black_path_count(rb_node* node); // TODO:

#endif // CFS_H