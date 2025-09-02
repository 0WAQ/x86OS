#include "common/macros.h"
#include "common/types.h"
#include "task/task_t.h"
#include "task/cfs_t.h"
#include "task/cfs.h"
#include "tools/rbtree.h"
#include "tools/log.h"

/**
 * @brief 权重表
 */
const u32_t sched_nice_to_weight[40] = {
 /* -20 */     88761,     71755,     56483,     46273,     36291,
 /* -15 */     29154,     23254,     18705,     14949,     11916,
 /* -10 */      9548,      7620,      6100,      4904,      3906,
 /*  -5 */      3121,      2501,      1991,      1586,      1277,
 /*   0 */      1024,       820,       655,       526,       423,
 /*   5 */       335,       272,       215,       172,       137,
 /*  10 */       110,        87,        70,        56,        45,
 /*  15 */        36,        29,        23,        18,        15,
};

void cfs_init(cfs_rq_t* rq) {
    rq->rb_root = RB_ROOT;
    rq->rb_leftmost = NULL;
    rq->min_vruntime = 0;
    rq->nr = 0;
    rq->total_weight = 0;
}

void cfs_enqueue_task(cfs_rq_t* rq, task_t* p) {
    sched_entity_t* se = &p->sched_entity;
    rb_node** link = &rq->rb_root.rb_node;
    rb_node* parent = NULL;
    sched_entity_t* entry = NULL;
    bool is_leftmost = 1;

    u32_t key = se->vruntime;
    while (*link) {
        parent = *link;
        entry = rb_entry(parent, sched_entity_t, cfs_node);

        // TODO:
        if (key < entry->vruntime) {
            link = &parent->rb_left;
        }
        else {
            link = &parent->rb_right;
            is_leftmost = 0;
        }
    }

    if (is_leftmost) {
        rq->rb_leftmost = &se->cfs_node;
    }

    // 插入并平衡节点
    rb_link_node(&se->cfs_node, parent, link);
    rb_insert_color(&se->cfs_node, &rq->rb_root);

    rq->min_vruntime = (rq->min_vruntime < se->vruntime) ? rq->min_vruntime : se->vruntime;
    rq->nr++;
    se->on_rq = true;
}

task_t* cfs_dequeue_task(cfs_rq_t* rq) {
    if (rq->nr == 0) {
        return NULL;
    }

    rb_node* node = rq->rb_leftmost;
    sched_entity_t* se = container_of(node, sched_entity_t, cfs_node);
    task_t* p = container_of(se, task_t, sched_entity);

    if (rq->nr > 1) {
        rq->rb_leftmost = rb_next(node);
    }
    else {
        rq->rb_leftmost = NULL;
    }

    rb_erase_next(node, &rq->rb_root);

    rq->nr--;
    se->on_rq = false;
    return p;
}

void cfs_erase_task(cfs_rq_t* rq, task_t* p) {
    if (p == NULL || rq->nr == 0) {
        return;
    }

    sched_entity_t* se = &p->sched_entity;
    rb_node* node = &se->cfs_node;

    rb_erase_next(node, &rq->rb_root);

    if (rq->nr > 0) {
        rq->rb_leftmost = rb_first(&rq->rb_root);
    }
    else {
        rq->rb_leftmost = NULL;
    }

    rq->nr--;
    se->on_rq = false;
}

task_t* cfs_pick_next_task(cfs_rq_t* rq) {
    rb_node* leftmost = rq->rb_leftmost;
    sched_entity_t* se = rb_entry(leftmost, sched_entity_t, cfs_node);
    return entry_of(se, task_t, sched_entity);
}

u32_t cfs_calc_delta_fair(u32_t delta_exec, u32_t load_weight) {
    /**
     * 举个例子, 若任务A的权重为 2048, 即 2 倍与标准任务(1024)
     *          并且该任务的执行时间(delta_exec)为 100
     *          
     * 那么 delta_vruntime = 100 * (2048 / 1024) = 200
     */
    // TODO: 实际上应该是 NICE_0_LOAD / load_weight, 不过 myos 没有开启浮点计算..
    return delta_exec * (load_weight / NICE_0_LOAD);
}

void cfs_update_vruntime(sched_entity_t* se, u32_t delta_exec) {
    u32_t delta_vruntime = cfs_calc_delta_fair(delta_exec, se->load.weight);
    se->vruntime += delta_vruntime;
}

void cfs_update_load_set(load_weight_t* lw, u32_t w) {
    lw->weight = w;
    lw->inv_weight = 0; // TODO:
}

void cfs_update_load_add(load_weight_t* lw, u32_t inc) {
    lw->weight += inc;
    lw->inv_weight = 0; // TODO:
}

void cfs_update_load_sub(load_weight_t* lw, u32_t dec) {
    lw->weight -= dec;
    lw->inv_weight = 0; // TODO:
}

bool cfs_rq_empty(cfs_rq_t* rq) {
    return (rq->nr == 0);
}

bool rb_check(rb_root* root, u32_t nr) {
    int cnt = 0, blacks = 0, no_warn = 1;

    rb_node* node;
    u32_t prev_key = 0;
    for(node = rb_first(root); node != NULL; node = rb_next(node)) {
        sched_entity_t* se = rb_entry(node, sched_entity_t, cfs_node);

        if(se->vruntime < prev_key) {
            log_print("[WARN] key(%d) < prev_key(%d).\n", se->vruntime, prev_key);
            no_warn = 0;
        }

        if(rb_is_red(node) && (!rb_parent(node) || rb_is_red(rb_parent(node)))) {
            log_print("[WARN] two red ndoes.\n");
            no_warn = 0;
        }

        if(cnt == 0) {
            blacks = rb_black_path_count(node);
        }
        else if((!node->rb_left || !node->rb_right) && (blacks != rb_black_path_count(node))) {
            log_print("[WARN] black count wrongs.\n");
            no_warn = 0;
        }

        prev_key = se->vruntime;
        cnt++;
    }

    if(no_warn) {
        log_print("rbtree with %d nodes no warn.\n", nr);
    }
    return no_warn;
}

int rb_black_path_count(rb_node* node) {
    int blacks;
    for (blacks = 0; node; node = rb_next(node)) {
        blacks += !rb_is_red(node);
    }
    return blacks;
}