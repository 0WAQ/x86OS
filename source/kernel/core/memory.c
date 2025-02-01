/***
 * 
 * 内存管理
 * 
 */
#include "core/memory.h"
#include "dev/console.h"
#include "tools/log.h"
#include "tools/klib.h"

//////////////////////////////////////////////////////////////////////////////
    // 链接脚本kernek.lds定义的变量
    extern u8_t* mem_free_start;     // 为内核结束后的第一个字节
    extern u8_t s_text[], e_text[], s_data[], e_data[];
    extern u8_t kernel_base[];

    // 全局物理内存分配器, 其管理1MB以上的所有物理内存
    addr_alloc_t paddr_alloc;

    // 内核页目录表
    static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE)));

    // 地址映射表, 用于建立内核级的地址映射, 直接映射区
    static memory_map_t kernel_map[] = {
        {kernel_base,  s_text,                      0,        PTE_W},   // 内核栈区, 64KB以下
        {s_text,       e_text,                      s_text,   0},       // 内核代码区, 从64KB开始
        {s_data,       (void*)(MEM_EBDA_START - 1), s_data,   PTE_W},   // 内核数据区
        {(void*)CONSOLE_DISP_ADDR_START, (void*)CONSOLE_DISP_ADDR_END, (void*)CONSOLE_DISP_ADDR_START, PTE_W},  // 显存
        // 将1MB以后的所有内存也直接映射
        {(void*)MEM_EXT_START, (void*)MEM_EXT_END, (void*)MEM_EXT_START, PTE_W}
    };
//////////////////////////////////////////////////////////////////////////////

void memroy_init(boot_info_t* boot_info) {
    log_print("...Memory Init...");
    show_mem_info(boot_info);

    // 计算1MB以上的空闲内存容量, 并向下对齐到页大小的整数倍
    u32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START;
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE);
    log_print("Free Memory: start = 0x%x, size = 0x%x", MEM_EXT_START, mem_up1MB_free);

    u8_t* mem_free = (u8_t*)&mem_free_start;

    // 32位系统最多4GB内存
    // 最多需要 4*1024*1024*1024 / 4096 / 8 = 128KB大小的位图, 这里一次性分配
    // 将其放在mem_free_start处(在1MB下)


    // 初始化物理内存分配器, 让其管理从1MB开始的所有物理内存, 按页管理, 一页4096字节
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    // 放完之后, mem_free一定小于0x80000(显卡控制)
    ASSERT(mem_free < (u8_t*)MEM_EBDA_START);

    // 根据地址映射表, 创建内核页目录表
    create_kernel_table();

    // 加载cr3寄存器
    mmu_set_page_dir((u32_t)kernel_page_dir);
}

static void create_kernel_table() {

    // 清空页目录表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // 遍历地址映射表, 内核有五个地址映射关系
    int nr = sizeof(kernel_map) / sizeof(memory_map_t);
    for(int i = 0; i < nr; ++i) {
        memory_map_t* map = kernel_map + i;

        u32_t vstart = down2((u32_t)map->vstart, MEM_PAGE_SIZE);
        u32_t vend = up2((u32_t)map->vend, MEM_PAGE_SIZE);
        u32_t page_nr = (vend - vstart) / MEM_PAGE_SIZE;
    
        // 创建内核页表, 处理一个地址映射关系
        memory_create_map(kernel_page_dir, vstart, (u32_t)map->pstart, page_nr, map->perm);
    }
}

static int memory_create_map(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t page_nr, u32_t perm)
{
    // 遍历一个映射关系拥有的所有页, 在页目录表中建立映射
    for(int i = 0; i < page_nr; i++) {

        // 查找或创建一个页表项(页)
        pte_t* pte = find_pte(page_dir, vaddr, 1);
        if(pte == NULL) {
            return -1;
        }

        ASSERT(pte->present == 0);
        pte->v = paddr | perm | PTE_P;
    
        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }

    return 0;
}

u32_t memory_create_uvm() {
    
    // 分配一页物理内存, 用于放置用户的页目录表
    pde_t* page_dir = (pde_t*)addr_alloc_page(&paddr_alloc, 1);
    if(page_dir == 0) {
        return 0;
    }

    kernel_memset((void*)page_dir, 0, MEM_PAGE_SIZE);

    // 用户空间的虚拟地址最小为0x80000000
    // 页目录表中, 用户地址空间对应的索引位置, 即512及以后的页目录项都是用户的
    u32_t user_pde_start = pde_index(MEMORY_TASK_BASE);

    // 只复制内核的页目录表项, 以便与其它进程共享内核空间
    // 任务自己的内存空间在其加载时创建
    for(int i = 0; i < user_pde_start; i++) {
        page_dir[i].v = kernel_page_dir[i].v;
    }
    return (u32_t)page_dir;
}

int memory_alloc_page_for(u32_t vaddr, u32_t size, u32_t perm) {

    // 调用子函数, 为当前进程的页目录表分配表项(在tss_init中分配了内核的页表)
    return _memory_alloc_page_for(get_curr_task()->tss.cr3, vaddr, size, perm);
}

int _memory_alloc_page_for(u32_t page_dir, u32_t vaddr, u32_t size, u32_t perm) {
    u32_t curr_vaddr = vaddr;

    // 需要分配的页数
    u32_t page_count = up2(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    vaddr = down2(vaddr, MEM_PAGE_SIZE);

    // 逐一分配页, 而不是一次分配, 因为需要对每一页建立映射
    for(u32_t i = 0; i < page_count; ++i) {

        // 分配一个物理页
        u32_t paddr = memory_alloc_page();
        if(paddr == 0) {
            log_print("memory_alloc_page failed. no memory");
            return -1;
        }

        // 建立映射关系
        int errno = memory_create_map((pde_t*)page_dir, curr_vaddr, paddr, 1, perm);
        if(errno < 0) {
            log_print("memory_create_map failed. errno: %d", errno);
            
            // 将前边分配的页全部释放
            addr_free_page(&paddr_alloc, vaddr, i + 1);
            
            // 断开映射关系
            // TODO:

            return -1;
        }

        curr_vaddr += MEM_PAGE_SIZE;
    }

    return 0;
}

u32_t memory_alloc_page() {

    // 分配一页物理内存
    u32_t paddr = addr_alloc_page(&paddr_alloc, 1);
    return paddr;
}

void memory_free_page(u32_t addr) {

    // 若地址小于0x80000000, 说明是由memory_alloc_page分配的
    if(addr < MEMORY_TASK_BASE) {
        addr_free_page(&paddr_alloc, addr, 1);
    }

    // 否则就是由memory_alloc_page_for分配的
    else {
        // 获取addr对应的页表
        pte_t* pte = find_pte(curr_page_dir(), addr, 0);
        ASSERT(pte != NULL && pte->present);

        // 将其释放掉
        addr_free_page(&paddr_alloc, pte_addr(pte), 1);
        
        // 取消映射关系
        pte->v = 0;
    }
}


static void show_mem_info(boot_info_t* boot_info) {
    log_print("Memory Region:");
    for(int i = 0; i < boot_info->ram_region_count; i++) {
        log_print("  [%d]: 0x%x - 0x%x", i, 
            boot_info->ram_region_cfg[i].start, 
            boot_info->ram_region_cfg[i].size);
    }
    log_print("");
}

u32_t memory_copy_uvm(u32_t from, u32_t to) {

    // 复制用户空间的页表
    u32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t* pde = (pde_t*)from + user_pde_start; // 父进程用户空间的起始页目录项
    
    // 遍历父进程的用户空间的页目录项
    for(int i = user_pde_start; i < PDE_CNT; ++i, ++pde) {
        if(!pde->present) {
            continue;
        }

        // 起始页表项
        pte_t* pte = (pte_t*)pde_addr(pde);
        
        // 遍历该页目录项中的所有页表项
        for(int j = 0; j < PTE_CNT; ++j, ++pte) {
            if(!pte->present) {
                continue;
            }

            // 分配一个物理页作为页表项
            u32_t page = addr_alloc_page(&paddr_alloc, 1);
            if(page == 0) {
                goto copy_uvm_failed;
            }

            // 建立映射关系
            u32_t vaddr = (i << 22) | (j << 12);
            int ret = memory_create_map((pde_t*)to, vaddr, page, 1, get_pte_perm(pte));
            if(ret < 0) {
                goto copy_uvm_failed;
            }

            // 复制内容
            kernel_memcpy((void*)page, (void*)vaddr, MEM_PAGE_SIZE);
        }
    }

    return to;

copy_uvm_failed:

    if(to) {
        memory_destory_uvm(to);
    }

    return 0;
}

u32_t memory_share_uvm(u32_t from, u32_t to) {
    // 共享用户空间的页表
    u32_t user_pde_start = pde_index(MEMORY_TASK_BASE);

    pde_t* pde_from = (pde_t*)from + user_pde_start;
    pde_t* pde_to   = (pde_t*)to + user_pde_start;      // 从这里开始, 子进程的页目录项是空的

    // 遍历页目录项
    for(int i = user_pde_start; i < PDE_CNT; ++i, ++pde_from, ++pde_to) {
        // 若from没有该项, 则跳过
        if(!pde_from->present) {
            continue;
        }

        // 为子进程分配一页物理内存作为pde的地址
        u32_t to_page_table_addr = memory_alloc_page();
        if(to_page_table_addr == 0) {
            return -1;  // Out of memory
        }

        // 设置页目录项的属性
        pde_to->v = to_page_table_addr | PDE_P | PDE_U | PDE_W;
        
        // 遍历页表项
        pte_t* pte_from = (pte_t*)pde_addr(pde_from);
        pte_t* pte_to   = (pte_t*)to_page_table_addr;
        for(int j = 0; j < PTE_CNT; ++j, ++pte_from, ++pte_to) {
            // 若父进程的页表项没有映射, 则跳过
            if(!pte_from->present) {
                continue;
            }

            // 让页只读, 并且映射到相同的页
            pte_from->write_enable = 0;
            pte_to->v = pte_from->v;
        }
    }
    return 0;
}

void do_wp_page(u32_t addr) {

    task_t* task = get_curr_task();

    // 页目录表与目录表项
    u32_t dir_addr = task->tss.cr3;
    pde_t* pde = (pde_t*)dir_addr + pde_index(addr);

    // 页表与页表项
    u32_t table_addr = pde_addr(pde);
    pte_t* pte = (pte_t*)table_addr + pte_index(addr); 

    // 物理页
    u32_t old_page = pte_addr(pte);

    // 表示页面没有被共享 TODO:

    // 分配一页物理内存
    u32_t new_page = memory_alloc_page();
    ASSERT(new_page >= 0);

    // 减少引用计数
    // TODO:

    // 拷贝
    kernel_memcpy((void*)new_page, (void*)old_page, MEM_PAGE_SIZE);

    // 将页表映射到新的物理页面
    pte->v = new_page | PTE_P | PTE_W | PTE_U;
}

void memory_destory_uvm(u32_t page_dir) {
    u32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t* pde = (pde_t*)page_dir + user_pde_start;

    for(int i = user_pde_start; i < PDE_CNT; ++i, ++pde) {
        if(!pde->present) {
            continue;
        }

        pte_t* pte = (pte_t*)pde_addr(pde);
        for(int j = 0; j < PTE_CNT; ++j, ++pte) {
            if(!pte->present) {
                continue;
            }
            
            // 释放页
            addr_free_page(&paddr_alloc, pte_addr(pte), 1);
        }

        // 释放页目录项
        addr_free_page(&paddr_alloc, pde_addr(pde), 1);
    }

    // 释放页目录表
    addr_free_page(&paddr_alloc, page_dir, 1);
}

u32_t memory_vaddr_to_paddr(u32_t page_dir, u32_t vaddr) {
    
    // 
    pte_t* pte = find_pte((pde_t*)page_dir, vaddr, 0);
    if(pte == NULL) {
        return 0;
    }

    return pte_addr(pte) + (vaddr & (MEM_PAGE_SIZE - 1));
}

int memory_copy_uvm_data(u32_t to, u32_t page_dir, u32_t from, u32_t size) {
    while(size > 0) {
        u32_t to_paddr = memory_vaddr_to_paddr(page_dir, to);
        if(to_paddr == 0) {
            return -1;
        }

        u32_t offset = to_paddr & (MEM_PAGE_SIZE - 1);
        u32_t cur_size = MEM_PAGE_SIZE - offset;
        if(cur_size > size) {
            cur_size = size;
        }

        kernel_memcpy((void*)to_paddr, (void*)from, cur_size);
    
        size -= cur_size;
        to += cur_size;
        from += cur_size;
    }
}
