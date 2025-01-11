/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */

SECTIONS
{
    PROVIDE(kernel_base = 0x00);

    /* 内核的elf文件加载到0x100000处，但是入口函数的地址放在0x10000处 */
    . = 0x00010000;

    PROVIDE(s_text = .);
    .text : {
        *(EXCLUDE_FILE(*first_task* *lib_syscall*) .text)
    }
    .rodata : {
        *(EXCLUDE_FILE(*first_task* *lib_syscall*) .rodata)
    }
    PROVIDE(e_text = .);

    . = ALIGN(4096);
    PROVIDE(s_data = .);
    .data : {
        *(EXCLUDE_FILE(*first_task* *lib_syscall*) .data)
    }

    .bss : {
        *(EXCLUDE_FILE(*first_task* *lib_syscall*) .bss)
    }
    PROVIDE(e_data = .);

/*************************************************************/

    . = 0x80000000;

    PROVIDE(s_first_task = LOADADDR(.first_task));
    .first_task : AT(e_data) {
        *first_task_entry*(.text .data .rodata .data)
        *first_task*(.text .data .rodata .data)
        *lib_syscall*(.text .data .bss)
    }
    PROVIDE(e_first_task = LOADADDR(.first_task) + SIZEOF(.first_task));

    PROVIDE(mem_free_start = e_first_task);
}