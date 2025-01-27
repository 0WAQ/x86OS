/***
 * 
 * 位图
 * 
 */
#include "tools/bitmap.h"
#include "tools/klib.h"

void bitmap_init(bitmap_t* bitmap, u8_t* bits, u32_t bit_count, int val)
{
    bitmap->bit_count = bit_count;
    bitmap->bits = bits;

    u32_t bytes = bitmap_byte_count(bitmap->bit_count);
    val = val ? 0xFF : 0x00;
    kernel_memset(bitmap->bits, val, bytes);
}

int bitmap_get_bit(bitmap_t* bitmap, u32_t index)
{
    // 在第(index / 8)个字节 的 第(index % 8)个位置上
    return (bitmap->bits[index / 8] & (1 << (index % 8))) ? 1 : 0;
}

void bitmap_set_bit(bitmap_t* bitmap, u32_t index, u32_t count, int val)
{
    if(val) {
        for(u32_t i = 0; i < count && index < bitmap->bit_count; ++i, ++index)
            bitmap->bits[index / 8] |= (1 << (index % 8));
    }
    else {
        for(u32_t i = 0; i < count && index < bitmap->bit_count; ++i, ++index)
            bitmap->bits[index / 8] &= ~(1 << (index % 8));
    }
}

int bitmap_is_set(bitmap_t* bitmap, u32_t index)
{
    return bitmap_get_bit(bitmap, index) ? 1 : 0;
}

int bitmap_alloc_nbits(bitmap_t* bitmap, u32_t count, int val)
{
    int i = 0, len = 0;
    while(i < bitmap->bit_count) {
        if(bitmap_get_bit(bitmap, i) == val) {
            if(++len == count) {
                // 分配
                bitmap_set_bit(bitmap, i - len + 1, len, ~val);
                return i - len + 1;
            }
        }
        else {
            len = 0;
        }
        ++i;
    }

    return -1;
}

u32_t bitmap_byte_count(u32_t bit_count) {
    return (bit_count + 7) / 8;    // 向上取整
}