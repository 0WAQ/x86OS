#ifndef BITMAP_H
#define BITMAP_H

#include "common/types.h"
#include "tools/bitmap_t.h"

void bitmap_init(bitmap_t* bitmap, u8_t* bits, u32_t bit_count, int val);
int bitmap_get_bit(bitmap_t* bitmap, u32_t index);
void bitmap_set_bit(bitmap_t* bitmap, u32_t index, u32_t count, int val);

bool bitmap_is_set(bitmap_t* bitmap, u32_t index);
int bitmap_alloc_nbits(bitmap_t* bitmap, u32_t count, int val);

u32_t bitmap_byte_count(u32_t bit_count);

#endif // BITMAP_H