/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */

SECTIONS {

    PROVIDE(kernel_base = 0x0);

    /* 内核的elf文件加载到0x100000处，但是入口函数的地址放在0x10000处 */
    . = 0x00010000;

    PROVIDE(s_text = .);
    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }
    PROVIDE(e_text = .);

    . = ALIGN(4096);
    PROVIDE(s_data = .);
    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
    PROVIDE(e_data = .);

    PROVIDE(mem_free_start = .);
}