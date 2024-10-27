/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */

SECTIONS {

    /* 内核的elf文件加载到0x100000处，但是入口函数的地址放在0x10000处 */
    . = 0x00010000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    PROVIDE(mem_free_start = .);
}