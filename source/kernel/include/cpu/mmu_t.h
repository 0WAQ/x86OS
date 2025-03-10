/***
 * 
 * mmu基本类型
 * 
 */
#ifndef MMU_T_H
#define MMU_T_H

#include "common/types.h"

#define PDE_CNT     (1024)
#define PTE_CNT     (1024)

#define PDE_P       (1 << 0)
#define PTE_P       (1 << 0)

#define PDE_W       (1 << 1)
#define PTE_W       (1 << 1)

#define PDE_U       (1 << 2)
#define PTE_U       (1 << 2)

#pragma pack(1)

/**
 * @brief 页目录项
 */
typedef union _pde_t{
    u32_t v;
    struct {
        u32_t present : 1;
        u32_t write_enable : 1;
        u32_t user_mode_access : 1;
        u32_t write_through : 1;
        u32_t cache_disable : 1;
        u32_t accessed : 1;
        u32_t : 1;
        u32_t ps : 1;
        u32_t : 4;
        u32_t phy_pt_addr : 20;
    };
}pde_t;

/**
 * @brief 页表项
 */
typedef union _pte_t{
    u32_t v;
    struct {
        u32_t present : 1;
        u32_t write_enable : 1;
        u32_t user_mode_access : 1;
        u32_t write_through : 1;
        u32_t cache_disable : 1;
        u32_t accessed : 1;
        u32_t dirty : 1;
        u32_t pat : 1;
        u32_t global : 1;
        u32_t : 3;
        u32_t phy_page_addr : 20;
    };
}pte_t;

#pragma pack()

#endif // MMU_T_H
