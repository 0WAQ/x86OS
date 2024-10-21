/**
 * 
 * 内核初始化
 * 
 */
#include "init.h"
#include "common/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "core/task.h"
#include "os_cfg.h"

// 该函数返回后，会长跳转至gdt_reload，并跳转到init_main
void kernel_init(boot_info_t* boot_info) {

    ASSERT(boot_info->ram_region_count != 0);

    // 初始化cpu，并重新加载gdt
    cpu_init();

    // 初始化日志
    log_init();

    // 初始化idt
    irq_init();

    // 初始化定时器
    time_init();
}

static task_t task1, task2;
static uint32_t task2_stack[1024];

void task2_entry() {
    int cnt = 0;
    for(;;) {
        log_print("task2: %d", cnt++);
        task_switch_from_to(&task2, &task1);
    }
}

void list_test() {
    list_t list;
    list_node_t nodes[5];


    // 测试头插
    list_init(&list);
    log_print("list: first = 0x%x, last = 0x%x, count = %d",
        list_first(&list), list_last(&list), list_count(&list));
    for(int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_print("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);
    }
    log_print("list: first = 0x%x, last = 0x%x, count = %d",
        list_first(&list), list_last(&list), list_count(&list));

    // 测试尾插
    list_init(&list);
    for(int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_print("insert last to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }
    log_print("list: first = 0x%x, last = 0x%x, count = %d",
        list_first(&list), list_last(&list), list_count(&list));

    // 测试头删
    for(int i = 0; i < 5; i++) {
        list_node_t* node = list_remove_first(&list);
        log_print("remove first from list: %d, 0x%x", i, (uint32_t)node);
    }
    log_print("list: first = 0x%x, last = 0x%x, count = %d",
        list_first(&list), list_last(&list), list_count(&list));

    // 测试任意位置删除
        list_init(&list);
        for(int i = 0; i < 5; i++) {
            list_node_t* node = nodes + i;
            log_print("insert last to list: %d, 0x%x", i, (uint32_t)node);
            list_insert_last(&list, node);
        }
        log_print("list: first = 0x%x, last = 0x%x, count = %d",
            list_first(&list), list_last(&list), list_count(&list));

    for(int i = 0; i < 5; i++) {
        list_node_t* node = nodes + i;
        log_print("remove node to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }


    struct task {
        int i;
        list_node_t list_node;
    };

    struct task v = {123, NODE_NULL};

    list_node_t* vnode = &v.list_node;
    struct task* x = list_entry_of(vnode, struct task);
    
    if(x->i == 123) {
        log_print("container_of correct!");
    }
    else {
        log_print("container_of wrong!");
    }
}

void init_main() {
    // irq_enalbe_global();

    // 测试链表
    list_test();

    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);

    // 这里不需要给参数，因为当cpu从当前任务切换走时，会将状态保存，只要保证有地方可去就行
    task_init(&task1, (uint32_t)0, 0);
    task_init(&task2, (uint32_t)task2_entry, (uint32_t)&task2_stack[1024]);

    // tr寄存器默认为空，加载task1到tr
    ltr(task1.tss_sel);  // 若使用task_switch_from_to使用switch_tss，那么需要启用改行

    int cnt = 0;
    for(;;) {
        log_print("task1: %d", cnt++);
        task_switch_from_to(&task1, &task2);
    }
}