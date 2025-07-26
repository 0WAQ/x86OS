/**
 * ELF相关头文件及配置
 */
#ifndef ELF_H
#define ELF_H

#include "types.h"


// ELF Header
#define EI_NIDENT   16
#define ELF_MAGIC   0x7F

// type
#define ET_NOTE     0   // No file type
#define ET_REL      1   // Relocatable file (可重定位文件)
#define ET_EXEC     2   // Executable file (可执行文件, 通常以 .o 结尾)
#define ET_DYN      3   // Shared object file (动态库文件)
#define ET_CORE     4   // Coredump file
#define ET_NUM      5
#define ET_LOPROC   0xff00
#define ET_HIPROC   0xffff

// machine
#define EM_NONE     0
#define EM_M32      1
#define EM_SPARC    2
#define EM_386      3   // 80386
#define EM_68K      4
#define EM_88K      5
#define EM_860      7
#define EM_MIPS     8
// ...

// version
#define EV_NONE     0
#define EV_CURRENT  1
#define EV_NUM      2


/**
 * @brief ELF头表(ELF Header Table)
 */
typedef struct {
    u8_t e_ident[EI_NIDENT];    // Magic Number
    u16_t e_type;           // 文件的类型
    u16_t e_machine;        // 文件的架构
    u32_t e_version;        // 版本号
    u32_t e_entry;          // 文件入口地址
    u32_t e_phoff;          // Program Header Table 的 Offset
    u32_t e_shoff;          // Section Header Table 的 Offset
    u32_t e_flags;          // 特定于处理器的标志, 一般为 0
    u16_t e_ehsize;         // Elf header的大小
    u16_t e_phentsize;      // Program Header 的入口大小
    u16_t e_phnum;          // Program Header Table 的数量
    u16_t e_shentsize;      // Section Header 的入口大小
    u16_t e_shnum;          // Section Header Table 的数量
    u16_t e_shstrndx;       // xxx Section Header 中的字符串表索引
} __attribute__((packed)) Elf32_Ehdr;


////////////////////////////////////////////////////////////////////////


// TODO: 除PT_LOAD外都暂且没用到
#define PT_NULL     0
#define PT_LOAD     1   // 可加载类型 
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6


/**
 * @brief 程序头表(Program Header Table)
 */
typedef struct {
    u32_t p_type;       // 
    u32_t p_offset;     // 
    u32_t p_vaddr;      // 
    u32_t p_paddr;      // 
    u32_t p_filesz;     // 
    u32_t p_memsz;      // 
    u32_t p_flags;      // 
    u32_t p_align;      // 
} __attribute__((packed)) Elf32_Phdr;

#endif // ELF_H