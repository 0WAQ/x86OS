/**
 *
 * 32位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 */

#include "loader.h"
#include "loader_32.h"

void loader_kernel() {

    // 从第100个扇区开始读取扇区，将内核映像文件(elf格式)加载到1MB处
    read_disk(100, 500, (uint8_t*)SYS_KERNEL_LOAD_ADDR);

    // 读取kernel的elf文件，而不是bin文件
    // 需要先将其从内存中读取出来解析，获得其入口函数地址0x10000(在lds中设置)
    uint32_t kernel_entry = reload_elf_file((uint8_t*)SYS_KERNEL_LOAD_ADDR);
    if(kernel_entry == 0) {
        die(-1);
    }

    // 打开分页机制
    enable_page_mode();

    // 跳转到内核代码
    ((void(*)(boot_info_t*))kernel_entry)(&boot_info);
    
    for(;;);
}

void read_disk(uint32_t sector, uint32_t sector_cnt, uint8_t* buffer) {
    
    // 读取LBA参数
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_cnt >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_cnt));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

    // 读取数据
    uint16_t* buf = (uint16_t*)buffer;
    while(sector_cnt-- > 0) {

        // 读前检查，等待数据就位
        while((inb(0x1F7) & 0x88) != 0x08);

        // 读取数据并将其写入到缓存中
        for(int i = 0; i < SECTOR_SIZE / 2; i++) {
            *buf++ = inw(0x1F0);
        }
    }
}

uint32_t reload_elf_file(uint8_t* file_buffer) {

    // 从0x100000处开始的数据转换成elf文件，elf不像bin一样可以直接运行，需要先解析
    Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)file_buffer;
    
    // 判断文件是否是合格的elf
    if(elf_hdr->e_ident[0] != 0x7F || 
       elf_hdr->e_ident[1] != 'E'  ||
       elf_hdr->e_ident[2] != 'L'  ||
       elf_hdr->e_ident[3] != 'F')
    {
        return 0;
    }

    // 加载程序头，将内容拷贝到相应的位置
    for(int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)(file_buffer + elf_hdr->e_phoff) + i;
        if(phdr->p_type != PT_LOAD) {
            continue;
        }

        // 全部使用物理地址
        uint8_t* src = file_buffer + phdr->p_offset;
        uint8_t* dest = (uint8_t*)phdr->p_paddr;
        for(int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

        // memsz和filesz不同时要填0
        dest = (uint8_t*)phdr->p_paddr + phdr->p_filesz;
        for(int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
            *dest++ = 0;
        }
    }

    return elf_hdr->e_entry;
}

void enable_page_mode() {
    static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };

    uint32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE);
    write_cr3((uint32_t)page_dir);

    write_cr0(read_cr0() | CR0_PG);
}

void die(int err_code) {
    for(;;);
}