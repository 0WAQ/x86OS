/***
 * 
 * mmu
 * 
 */
#include "cpu/mmu.h"
#include "core/memory.h"
#include "tools/klib.h"

// 外部的地址分配结构, 在memory.c中定义 
extern addr_alloc_t paddr_alloc;

pte_t* find_pte(pde_t* page_dir, uint32_t vaddr, int is_alloc) {

    pte_t* ret_pte;

    // vaddr对应的页目录项
    pde_t* pde = page_dir + pde_index(vaddr);

    // 页目录项是否存在
    if(pde->present) {
        ret_pte = (pte_t*)pde_addr(pde);
    }
    else {
        // 如果不存在, 则考虑分配一个
        if(is_alloc == 0) {
            return (pte_t*)0;
        }

        // 分配一个物理页表, 当作一个页目录项(页目录项的大小正好是一页)
        uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1);
        if(pg_paddr == 0) {
            return (pte_t*)0;
        }

        // 更新对应的页目录项
        pde->v = pg_paddr | PTE_P | PDE_W | PDE_U;

        ret_pte = (pte_t*)pg_paddr;
        kernel_memset(ret_pte, 0, MEM_PAGE_SIZE);
    }

    // 返回对应的页表项
    return ret_pte + pte_index(vaddr);
}