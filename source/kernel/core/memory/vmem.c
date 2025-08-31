/**
 * 虚拟内存管理
 */
#include "common/types.h"
#include "core/memory/pmem.h"
#include "core/memory/vmem.h"
#include "core/task/task.h"
#include "cpu/mmu.h"
#include "tools/log.h"
#include "tools/klib.h"

extern pde_t kernel_page_dir[PDE_CNT];

int vmem_map_page(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t perm) {
    pte_t* pte = find_pte(page_dir, vaddr, true);
    if(pte == NULL) {
        return -1;
    }
    ASSERT(pte->present == 0);
    pte->v = (paddr | perm | PTE_P);
    return 0;
}

int vmem_map_pages(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t page_nr, u32_t perm) {
    // 遍历一个映射关系拥有的所有页, 在页目录表中建立映射
    for(u32_t i = 0; i < page_nr; ++i) {
        if (vmem_map_page(page_dir, vaddr, paddr, perm) < 0) {
            return -1;
        }
        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }
    return 0;
}

int vmem_unmap_page(pde_t* page_dir, u32_t vaddr) {
    pte_t* pte = find_pte(page_dir, vaddr, false);
    if (pte == NULL || pte->present == 0) {
        return -1;
    }

    u32_t paddr = pte_addr(pte);
    pte->v = 0;

    flush_tlb(vaddr);
    return 0;
}

int vmem_unmap_pages(pde_t* page_dir, u32_t vaddr, u32_t page_nr) {
    vaddr = kdown2(vaddr, MEM_PAGE_SIZE);

    int count = 0;
    for (u32_t i = 0; i < page_nr; ++i) {
        if (vmem_unmap_page(page_dir, vaddr) >= 0) {
            ++count;
        }
        vaddr += MEM_PAGE_SIZE;
    }
    return count;
}

int vmem_unmap_range(pde_t* page_dir, u32_t vstart, u32_t size) {
    u32_t vend = vstart + size;

    u32_t addr = kdown2(vstart, MEM_PAGE_SIZE);
    u32_t end = kup2(vend, MEM_PAGE_SIZE);

    int count = 0;
    while (addr < end) {
        if (vmem_unmap_page(page_dir, addr) >= 0) {
            ++count;
        }
        addr += MEM_PAGE_SIZE;
    }
    return count;
}

int vmem_alloc_page(u32_t page_dir, u32_t vaddr, u32_t size, u32_t perm) {
    u32_t curr_vaddr = vaddr;

    // 需要分配的页数
    u32_t page_count = kup2(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    vaddr = kdown2(vaddr, MEM_PAGE_SIZE);

    // 逐一分配页, 而不是一次分配, 因为需要对每一页建立映射
    for(u32_t i = 0; i < page_count; ++i) {
        // 分配一个物理页
        u32_t paddr = pmem_alloc_page();
        if(paddr == 0) {
            log_print("pmem_alloc_page failed. no memory");
            return -1;
        }

        // 建立映射关系
        int errno = vmem_map_page((pde_t*)page_dir, curr_vaddr, paddr, perm);
        if(errno < 0) {
            log_print("vmem_map_page failed. errno: %d", errno);
            
            // 将前边分配的页全部释放
            pmem_free_pages(vaddr, i + 1);
            return -1;
        }
        curr_vaddr += MEM_PAGE_SIZE;
    }
    return 0;
}

// TODO:
void vmem_free_page(u32_t vaddr) {
    // 若地址小于 0x80000000, 说明是由 pmem_alloc_page 分配的
    if(vaddr < MEMORY_TASK_BASE) {
        pmem_free_page(vaddr);
    }

    // 否则就是由 vmem_alloc_page 分配的
    else {
        // 获取 vaddr 对应的页表
        pte_t* pte = find_pte(get_curr_page_dir(), vaddr, false);
        ASSERT(pte != NULL && pte->present);

        // 将其释放掉
        pmem_free_page(pte_addr(pte));
        
        // 取消映射关系
        pte->v = 0;
    }
}

u32_t vmem_create_uvms() {
    // 分配一页物理内存, 用于放置用户的 pd
    pde_t* page_dir = (pde_t*)pmem_alloc_page();
    if(page_dir == 0) {
        return 0;
    }

    kmemset((void*)page_dir, 0, MEM_PAGE_SIZE);

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

void vmem_destory_uvms(u32_t page_dir) {
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
            pmem_free_page(pte_addr(pte));
        }

        // 释放页目录项
        pmem_free_page(pde_addr(pde));
    }

    // 释放页目录表
    pmem_free_page(page_dir);
}

u32_t vmem_copy_uvms(u32_t from, u32_t to) {
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
            u32_t page = pmem_alloc_page();
            if(page == 0) {
                goto copy_uvm_failed;
            }

            // 建立映射关系
            u32_t vaddr = (i << 22) | (j << 12);
            int ret = vmem_map_page((pde_t*)to, vaddr, page, get_pte_perm(pte));
            if(ret < 0) {
                goto copy_uvm_failed;
            }

            // 复制内容
            kmemcpy((void*)page, (void*)vaddr, MEM_PAGE_SIZE);
        }
    }

    return to;

copy_uvm_failed:
    if(to) {
        vmem_destory_uvms(to);
    }
    return 0;
}

int vmem_copy_corss_uvms(u32_t to, u32_t page_dir, u32_t from, u32_t size) {
    while(size > 0) {
        u32_t to_paddr = vmem_vaddr_to_paddr(page_dir, to);
        if(to_paddr == 0) {
            return -1;
        }

        u32_t offset = to_paddr & (MEM_PAGE_SIZE - 1);
        u32_t cur_size = MEM_PAGE_SIZE - offset;
        if(cur_size > size) {
            cur_size = size;
        }

        kmemcpy((void*)to_paddr, (void*)from, cur_size);
    
        size -= cur_size;
        to += cur_size;
        from += cur_size;
    }
    return 0;
}

u32_t vmem_share_uvms(u32_t from, u32_t to) {
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
        u32_t to_page_table_addr = pmem_alloc_page();
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

void vmem_do_wp_page(u32_t addr) {
    task_t* task = get_curr_task();

    // 页目录表与目录表项
    u32_t dir_addr = task->tss.cr3;
    pde_t* pde = (pde_t*)dir_addr + pde_index(addr);

    // 页表与页表项
    u32_t table_addr = pde_addr(pde);
    pte_t* pte = (pte_t*)table_addr + pte_index(addr); 

    // 物理页
    u32_t old_page = pte_addr(pte);

    // 表示页面没有被共享
    // TODO:

    // 分配一页物理内存
    u32_t new_page = pmem_alloc_page();
    ASSERT(new_page != 0);

    // 减少引用计数
    // TODO:

    // 拷贝
    kmemcpy((void*)new_page, (void*)old_page, MEM_PAGE_SIZE);

    // 将页表映射到新的物理页面
    pte->v = new_page | PTE_P | PTE_W | PTE_U;
}

u32_t vmem_vaddr_to_paddr(u32_t page_dir, u32_t vaddr) {
    // 
    pte_t* pte = find_pte((pde_t*)page_dir, vaddr, false);
    if(pte == NULL) {
        return 0;
    }
    return pte_addr(pte) + (vaddr & (MEM_PAGE_SIZE - 1));
}
