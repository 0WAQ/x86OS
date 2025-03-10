/**
 * 
 * ELF相关头文件及配置
 * 
 */
#ifndef ELF_H
#define ELF_H

#include "types.h"

// ELF相关基本类型
typedef u32_t Elf32_Addr;
typedef u16_t Elf32_Half;
typedef u32_t Elf32_Off;
typedef u32_t Elf32_Sword;
typedef u32_t Elf32_Word;

#pragma pack(1)

// ELF Header
#define EI_NIDENT   16
#define ELF_MAGIC   0x7F

#define ET_EXEC     2   // 可执行文件
#define ET_386      3   // 80386

typedef struct {
    char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}Elf32_Ehdr;

#define PT_LOAD     1   // 可加载类型

typedef struct {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
}Elf32_Phdr;

#pragma pack()

#endif // ELF_H