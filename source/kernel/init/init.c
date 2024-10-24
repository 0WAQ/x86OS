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

    // 初始化任务管理器
    task_manager_init();
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
    struct task* x = list_entry_of(vnode, struct task, list_node);
    
    if(x->i == 123) {
        log_print("container_of correct!");
    }
    else {
        log_print("container_of wrong!");
    }
}

static task_t task2;
static uint32_t task2_stack[1024];

void task2_entry() {
    int cnt = 0;
    for(;;) {
        log_print("%s: %d", task2.name, cnt++);
        // sys_yield();
    }
}

void init_main() {
    // 测试链表
    // list_test();

    log_print("...kernel is running...");
    log_print("Version: %s", OS_VERSION);

    // 初始化第一个任务
    first_task_init();

    // 初始化第二个任务
    task_init(&task2, "task2", (uint32_t)task2_entry, (uint32_t)&task2_stack[1024]);

    // 开中断
    irq_enalbe_global();

    int cnt = 0;
    for(;;) {
        log_print("%s: %d", get_first_task()->name, cnt++);
        // sys_yield();
    }
}