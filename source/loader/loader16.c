__asm__(".code16gcc");

#include "common/asm.h"
#include "loader/loader.h"

boot_info_t boot_info;

extern u32_t loader32_protect_mode_entry();

void loader16_entry() {
    loader16_show_msg("...Loading...\r\n");
    loader16_detect_memory();
    loader16_enter_protect_mode();
}

// 参考：https://wiki.osdev.org/Memory_Map_(x86)
// 1MB以下比较标准, 在1M以上会有差别:
// 检测：https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15.2C_AH_.3D_0xC7
void loader16_detect_memory() {
    loader16_show_msg("...Detecting Memory...");
    
    u32_t contID = 0;
    SMAP_entry_t SMAP_entry;
    int signature = 0, bytes = 0;

    int len = 0;
    for(int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
        __asm__ __volatile__(
            "int $0x15" 
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(&SMAP_entry)
        );

        if(signature != 0x534D4150) {
            loader16_show_msg("Failed!\n\r");
            return;
        }

        // ACPI的位0的值为0时忽略
        if(bytes > 20 && (SMAP_entry.ACPI & 0x0001) == 0) {
            continue;
        }

        // 当type为1时，该内存区域可用，将其保存到boot_info的ram_region_cfg中
        if(SMAP_entry.type == 1) {
            // 只读取低32位
            boot_info.ram_region_cfg[len].start = SMAP_entry.base_low;
            boot_info.ram_region_cfg[len].size = SMAP_entry.length_low;
            ++len;
        }

        if(contID == 0) {
            break;
        }
    }
    boot_info.ram_region_count = len;

    loader16_show_msg("Successful!\r\n");
}

void loader16_enter_protect_mode() {
    loader16_show_msg("...Entering Protect Mode...\n\r");

    // 关中断
    cli();

    // 开启A20地址线
    // 使用的是Fast A20 Gate方式，见https://wiki.osdev.org/A20#Fast_A20_Gate
    u8_t data = inb(0x92);
    outb(0x92, data | 0x2);

    // 临时的GDT表
    u16_t gdt_table[][4] = {
        {0, 0, 0, 0},
        {0xFFFF, 0x0000, 0x9A00, 0x00CF},
        {0xFFFF, 0x0000, 0x9200, 0x00CF},
    };

    // 加载GDT；由于中断已经关闭，无需加载IDT
    lgdt((u32_t)gdt_table, sizeof(gdt_table));

    // 开启CR0的PE位
    u32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));

    // 远跳转
    far_jump(8, (u32_t)loader32_protect_mode_entry);
}

void loader16_show_msg(const char* msg) {
    char c;
    while((c = *msg++) != '\0') {
        __asm__ __volatile__(
            "mov $0xe, %%ah""\n\t"
            "mov %[ch], %%al""\n\t"
            "int $0x10"
            :                   // 输出操作数，无
            : [ch]"r"(c)        // 输入操作数，ch建立关联，"r"表示任意寄存器
        );
    }
}