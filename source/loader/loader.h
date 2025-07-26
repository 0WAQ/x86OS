#ifndef LOADER_H
#define LOADER_H

#include "common/types.h"
#include "loader/loader_t.h"

void loader16_entry();
void loader16_detect_memory();
void loader16_enter_protect_mode();
void loader16_show_msg(const char* msg);

void loader32_enter_kernel();
void loader32_read_disk(u32_t sector, u32_t sector_cnt, u8_t* buffer);
u32_t loader32_reload_elf_file(u8_t* buf);
void loader32_enable_page_mode();
void loader32_die(int err_code);

#ifndef LOADER_T_H
#endif // LOADER_T_H

#endif // LOADER_H