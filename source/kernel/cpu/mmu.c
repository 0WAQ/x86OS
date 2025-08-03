#include "cpu/mmu.h"
#include "core/memory/pmem.h"
#include "tools/klib.h"

pte_t* find_pte(pde_t* page_dir, u32_t vaddr, bool is_alloc) {
    pde_t* pde = page_dir + pde_index(vaddr);
    if (pde->present) {
        return (pte_t*)((u32_t*)pde_addr(pde) + pte_index(vaddr));
    }
    else if (is_alloc) {
        // 分配一个物理页作为 pt
        u32_t pt_paddr = pmem_alloc_page();
        if (pt_paddr == 0) {
            return NULL;
        }
        kernel_memset((void*)pt_paddr, 0, MEM_PAGE_SIZE);

        // 更新 pde
        pde->v = pt_paddr | PDE_P | PDE_W | PDE_U;
        return (pte_t*)((u32_t*)pde_addr(pde) + pte_index(vaddr));
    }
    return NULL;
}