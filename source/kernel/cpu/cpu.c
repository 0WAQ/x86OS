/**
 * 
 * CPU设置
 * 
 */
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "core/syscall.h"
#include "ipc/mutex.h"
#include "common/cpu_instr.h"
#include "os_cfg.h"

// 全局段描述符表
// static 
segment_desc_t gdt_table[GDT_TABLE_SIZE];

// 互斥锁
static mutex_t mutex;

void cpu_init() {
    // 初始化锁
    mutex_init(&mutex);

    // 初始化gdt, 让内核运行在平坦模型上
    gdt_init();
}

void gdt_init() {
    for(int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i << 3, 0, 0, 0);
    }

    // 平坦模型的数据段
    segment_desc_set(KERNEL_SELECTOR_DS, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL0 | DESC_ATTR_S_USR | DESC_ATTR_D |
        DESC_ATTR_TYPE_DATA | DESC_ATTR_TYPE_RW
    );

    // 平坦模型的代码段
    segment_desc_set(KERNEL_SELECTOR_CS, 0x00000000, 0xFFFFFFFF, 
        DESC_ATTR_P | DESC_ATTR_DPL0 | DESC_ATTR_S_USR | DESC_ATTR_D |
        DESC_ATTR_TYPE_CODE | DESC_ATTR_TYPE_ER
    );

    // 设置调用门
    gate_desc_set((gate_desc_t*)(gdt_table + (SELECTOR_SYSCALL >> 3)),
        KERNEL_SELECTOR_CS, (uint32_t)exception_handler_syscall, 
        GATE_ATTR_P | GATE_ATTR_DPL3 | GATE_ATTR_TYPE_SYSCALL | SYSCALL_PARAM_COUNT 
    );

    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t* desc = gdt_table + (selector >> 3); // 每个gdt的大小是8字节(64位)
    
    // 若limit超过20位，则界限位G为1
    if(limit > 0xFFFFF) {
        attr |= 0x8000;
        limit >> 12;
    }
    
    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
    desc->base24_31 = (base >> 24) & 0xFF;
}

void gate_desc_set(gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr) {
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
}

int gdt_alloc_desc() {

    /////////////////////////////////////////// 上锁
    mutex_lock(&mutex);

    for(int i = 1; i < GDT_TABLE_SIZE; ++i) {
        segment_desc_t* desc = gdt_table + i;
        if(desc->attr == 0) {
            mutex_unlock(&mutex);       ///////////////////////// 解锁
            return i * sizeof(segment_desc_t);
        }
    }


    mutex_unlock(&mutex);
    /////////////////////////////////////////// 解锁

    return -1;
}

void gdt_free_desc(int tss_sel) {
    
    /////////////////////////////////////////// 上锁
    mutex_lock(&mutex); 

    gdt_table[tss_sel / sizeof(segment_desc_t)].attr = 0;

    mutex_unlock(&mutex);
    /////////////////////////////////////////// 解锁

}

void switch_to_tss(int tss_sel) {

    // cpu发现跳转的目标是tss段，所以会将当前运行状态保存到当前tss段中(当前tss的值在tr中)
    // 并将目标tss段切换出来，然后接着运行(读取切换后的tss中的cs:ip)
    far_jump(tss_sel, 0);
}