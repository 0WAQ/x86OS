#include "common/asm.h"
#include "common/elf.h"
#include "common/boot_info.h"
#include "common/types.h"
#include "loader/loader.h"

extern boot_info_t boot_info;

void loader32_enter_kernel() {
    // 从第 100 个扇区开始读取扇区, 将内核映像文件(elf格式)加载到 内存地址 1MB 处
    loader32_read_disk(100, 500, (u8_t*)SYS_KERNEL_LOAD_ADDR);

    // 解析 elf 文件, 入口地址应为 0x10000
    u32_t kernel_entry = loader32_reload_elf_file((u8_t*)SYS_KERNEL_LOAD_ADDR);
    if(kernel_entry == 0) {
        loader32_die(-1);
    }

    // 打开分页机制
    loader32_enable_page_mode();

    // 跳转到内核代码
    ( (void(*)(boot_info_t*)) kernel_entry)(&boot_info);
}

void loader32_read_disk(u32_t sector, u32_t sector_cnt, u8_t* buffer) {
    // 读取LBA参数
    outb(0x1F6, (u8_t) (0xE0));

	outb(0x1F2, (u8_t) (sector_cnt >> 8));
    outb(0x1F3, (u8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (u8_t) (0));			    // LBA参数的32~39位
    outb(0x1F5, (u8_t) (0));				// LBA参数的40~47位

    outb(0x1F2, (u8_t) (sector_cnt));
	outb(0x1F3, (u8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (u8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (u8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (u8_t) 0x24);

    // 读取数据
    u16_t* buf = (u16_t*)buffer;
    while(sector_cnt-- > 0) {
        // 读前检查，等待数据就位
        while((inb(0x1F7) & 0x88) != 0x08);

        // 读取数据并将其写入到缓存中
        for(int i = 0; i < SECTOR_SIZE / 2; i++) {
            *buf++ = inw(0x1F0);
        }
    }
}

u32_t loader32_reload_elf_file(u8_t* buf) {
    // 从 0x100000 处开始的数据转换成elf文件，elf不像bin一样可以直接运行，需要先解析
    Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)buf;
    
    // 判断文件是否是合格的elf
    if(elf_hdr->e_ident[0] != 0x7F || 
       elf_hdr->e_ident[1] != 'E'  ||
       elf_hdr->e_ident[2] != 'L'  ||
       elf_hdr->e_ident[3] != 'F')
    {
        return 0;
    }

    // 读取Program Header，将内容拷贝到相应的位置
    for(int i = 0; i < elf_hdr->e_phnum; i++) {

        // 文件首字节 + Program Headers Offset是第一个phdr
        Elf32_Phdr* phdr = (Elf32_Phdr*)(buf + elf_hdr->e_phoff) + i;
        if(phdr->p_type != PT_LOAD) {
            continue;
        }

        // 全部使用物理地址, 此时分页机制还未打开
        u8_t* src = buf + phdr->p_offset;
        u8_t* dest = (u8_t*)phdr->p_paddr;
        for(int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

        // memsz和filesz不同时要填0(bss段)
        dest = (u8_t*)phdr->p_paddr + phdr->p_filesz;
        for(int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
            *dest++ = 0;
        }
    }

    return elf_hdr->e_entry;
}

void loader32_enable_page_mode() {
    // 页目录表
    static u32_t page_dir[1024] __attribute__((aligned(4096))) = {
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };

    u32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE);
    write_cr3((u32_t)page_dir);

    write_cr0(read_cr0() | CR0_PG);
}

void loader32_die(int err_code) {
    for(;;);
}