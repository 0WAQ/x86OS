#ifndef CFS_T_H
#define CFS_T_H

#include "common/types.h"
#include "tools/rbtree_t.h"

#define NICE_OFFSET     (40)
#define NICE_0_LOAD     (1024)      // 优先级为 0 的任务的权重

/**
 * @brief 权重相关
 */
typedef struct _load_weight_t {
    u32_t weight;       // 任务的权重
    u32_t inv_weight;   // TODO: 权重的倒数, 用以优化计算, 暂不使用
} load_weight_t;


/**
 * @brief cfs 运行队列
 */
typedef struct _cfs_rq_t {
    rb_root rb_root;
    rb_node* rb_leftmost;
    u32_t min_vruntime;
    u32_t nr;
    int total_weight;
} cfs_rq_t;


/**
 * @brief 调度实体, 目前只支持 cfs
 */
typedef struct _sched_entity_t {
    load_weight_t load;
    u32_t vruntime;
    rb_node cfs_node;
    bool on_rq;
} sched_entity_t;

#endif // CFS_T_H