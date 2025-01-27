/***
 * 
 * mmu相关头文件
 * 
 */
#ifndef MMU_H
#define MMU_H

#include "mmu_t.h"
#include "common/cpu_instr.h"


/**
 * @brief 查找或创建vaddr对应的页表项
 * @param is_alloc 当找不到pte时, 表示是否创建一个pte
 */
pte_t* find_pte(pde_t* page_dir, u32_t vaddr, int is_alloc);


// 重新加载CR3寄存器
static inline
void mmu_set_page_dir(u32_t paddr) {
    write_cr3(paddr);
}

// 根据虚拟地址, 获取页目录项的索引(一个小页表)
static inline
u32_t pde_index(u32_t vaddr) {
    return vaddr >> 22;
}

// 根据虚拟地址, 获取页表项的索引(页)
static inline
u32_t pte_index(u32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;
}

// 获取页目录项中的地址(即小页表的物理地址)
static inline
u32_t pde_addr(pde_t* pde) {
    return pde->phy_pt_addr << 12;
}

// 获取页表项中的地址(即页的物理地址)
static inline
u32_t pte_addr(pte_t* pte) {
    return pte->phy_page_addr << 12;
}

// 获取页表项的权限位
static inline
u32_t get_pte_perm(pte_t* pte) {
    return (pte->v & 0x1FF);
}

#endif // MMU_H
