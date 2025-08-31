#ifndef VMEM_H
#define VMEM_H

#include "common/types.h"
#include "task/task.h"
#include "cpu/mmu_t.h"

/**
 * @brief 创建 paddr 到 vaddr 的映射
 */
int vmem_map_page(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t perm);
int vmem_map_pages(pde_t* page_dir, u32_t vaddr, u32_t paddr, u32_t page_nr, u32_t perm);
int vmem_unmap_page(pde_t* page_dir, u32_t vaddr);
int vmem_unmap_pages(pde_t* page_dir, u32_t vaddr, u32_t page_nr);
int vmem_unmap_range(pde_t* page_dir, u32_t vstart, u32_t size);

/**
 * @brief 为指定页目录表分配虚拟内存
 */
int vmem_alloc_page(u32_t page_dir, u32_t vaddr, u32_t size, u32_t perm);
void vmem_free_page(u32_t vaddr); // TODO: To be changed

/**
 * @brief 为进程创建 vms
 */
u32_t vmem_create_uvms();

/**
 * @brief 销毁进程的 vms
 */
void vmem_destory_uvms(u32_t page_dir);

/**
 * @brief 为子进程拷贝父进程的代码和数据
 */
u32_t vmem_copy_uvms(u32_t page_dir, u32_t to_page_dir);

/**
 * @brief 跨页目录表拷贝数据
 */
int vmem_copy_corss_uvms(u32_t to, u32_t page_dir, u32_t from, u32_t size);

/**
 * @brief 共享页表
 */
u32_t vmem_share_uvms(u32_t from, u32_t to);

/**
 * @brief 写时复制
 */
void vmem_do_wp_page(u32_t address);

/**
 * @brief 将vaddr通过page_dir转换成物理地址
 */
u32_t vmem_vaddr_to_paddr(u32_t page_dir, u32_t vaddr);

/**
 * @brief 返回当前使用的页目录表
 */
static inline pde_t* get_curr_page_dir() {
    return (pde_t*)(get_curr_task()->tss.cr3);
}

#endif // VMEM_H