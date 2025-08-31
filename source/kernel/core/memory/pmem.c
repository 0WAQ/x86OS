/**
 * 物理内存管理
 */
#include "common/types.h"
#include "core/memory/pmem.h"
#include "core/memory/vmem.h"
#include "cpu/mmu.h"
#include "dev/console_t.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"
#include "tools/log.h"
#include "tools/klib.h"

//////////////////////////////////////////////////////////////////////////////

// 链接脚本 kernek.lds 定义的变量
extern u8_t* mem_free_start;     // 为内核结束后的第一个字节
extern u8_t s_text[], e_text[], s_data[], e_data[];
extern u8_t kernel_base[];

// 物理内存分配器, 管理 1MB 以上的所有物理内存
static paddr_alloc_t paddr_alloc;

// 地址映射表, 用于建立内核级的地址映射, 使用直接映射
static memory_map_t kernel_map[] = {
    // 内核栈区, 64KB以下
    { kernel_base, s_text,
          0, PTE_W
    },

    // 内核代码区, 从64KB开始
    { s_text, e_text,
          s_text, 0
    },

    // 内核数据区
    {
        s_data, (void*)(MEM_EBDA_START - 1),
        s_data, PTE_W
    },

    // 显存
    {
        (void*)CONSOLE_DISP_ADDR_START, (void*)CONSOLE_DISP_ADDR_END,
        (void*)CONSOLE_DISP_ADDR_START, PTE_W
    },

    // 1MB 以后的所有内存
    {
        (void*)MEM_EXT_START, (void*)MEM_EXT_END,
        (void*)MEM_EXT_START, PTE_W
    }
};

// 内核页目录表
pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE)));

//////////////////////////////////////////////////////////////////////////////

void memroy_init(boot_info_t* boot_info) {
    log_print("...Memory Init...");
    show_pmem_info(boot_info);

    // 计算 1MB 以上的空闲内存容量, 并向下对齐到页大小的整数倍
    u32_t tot_mem_up1MB = kdown2((total_pmem_size(boot_info) - MEM_EXT_START), MEM_PAGE_SIZE);
    log_print("Free Memory: start = 0x%x, size = 0x%x", MEM_EXT_START, tot_mem_up1MB);

    // 32 位系统最多 4GB 内存
    // 需要 4*1024*1024*1024 / 4096 / 8 = 128KB 大小的位图, 这里一次性分配
    // 将其放在 mem_free_start 处(目前在 1MB 下)
    u8_t* bitmap_start = (u8_t*)&mem_free_start;

    // 初始化物理内存分配器, 让其管理从1MB开始的所有物理内存, 按页管理, 一页4096字节
    paddr_alloc_init(&paddr_alloc, bitmap_start, MEM_EXT_START, tot_mem_up1MB, MEM_PAGE_SIZE);
    bitmap_start += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    // 确保不覆盖显卡区域 (0x80000)
    ASSERT(bitmap_start < (u8_t*)MEM_EBDA_START);

    // 根据 kernel_map, 建立内核页目录表
    create_kernel_table();

    // 加载 cr3 寄存器, 启用分页机制
    mmu_set_page_dir((u32_t)kernel_page_dir);
}

u32_t pmem_alloc_page() {
    u32_t paddr = paddr_alloc_pages(&paddr_alloc, 1);
    return paddr;
}

u32_t pmem_alloc_pages(u32_t nr) {
    u32_t paddr = paddr_alloc_pages(&paddr_alloc, nr);
    return paddr;
}

void pmem_free_page(u32_t addr) {
    paddr_free_pages(&paddr_alloc, addr, 1);
}

void pmem_free_pages(u32_t addr, u32_t nr) {
    paddr_free_pages(&paddr_alloc, addr, nr);
}

///////////////////////////////////////////////////////////////////////////////////////

static void create_kernel_table() {
    kmemset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // 遍历地址映射表, 内核有五个地址映射关系
    int nr = sizeof(kernel_map) / sizeof(memory_map_t);
    for(int i = 0; i < nr; ++i) {
        memory_map_t* curr_map = kernel_map + i;

        u32_t vstart = kdown2((u32_t)curr_map->vstart, MEM_PAGE_SIZE);
        u32_t vend = kup2((u32_t)curr_map->vend, MEM_PAGE_SIZE);
        u32_t page_nr = (vend - vstart) / MEM_PAGE_SIZE;
    
        // 创建映射
        int ret = vmem_map_pages(kernel_page_dir, vstart,
                (u32_t)curr_map->pstart, page_nr, curr_map->perm);
        ASSERT(ret >= 0);
    }
}

static void paddr_alloc_init(paddr_alloc_t* alloc, u8_t* bits, u32_t start,
                             u32_t size, u32_t page_size) {
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, size / page_size, 0);
}

static u32_t paddr_alloc_pages(paddr_alloc_t* alloc, u32_t nr) {
    u32_t addr = 0;
    mutex_lock(&alloc->mutex);
    int page_index = bitmap_alloc_nbits(&alloc->bitmap, nr, 0);
    mutex_unlock(&alloc->mutex);
    if(page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size;
    }
    return addr;
}

static void paddr_free_pages(paddr_alloc_t* alloc, u32_t addr, u32_t nr) {
    u32_t page_index = (addr - alloc->start) / alloc->page_size;
    mutex_lock(&alloc->mutex);
    bitmap_set_bit(&alloc->bitmap, page_index, nr, 0);
    mutex_unlock(&alloc->mutex);
}

static u32_t total_pmem_size(boot_info_t* boot_info) {
    u32_t tot = 0;
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        tot += boot_info->ram_region_cfg[i].size;
    }
    return tot;
}

static void show_pmem_info(boot_info_t* boot_info) {
    log_print("Memory Region:");
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        log_print("  [%d]: 0x%x - 0x%x", i, 
            boot_info->ram_region_cfg[i].start, 
            boot_info->ram_region_cfg[i].size);
    }
}