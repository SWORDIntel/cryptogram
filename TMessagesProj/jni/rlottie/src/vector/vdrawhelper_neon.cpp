#if defined(__ARM_NEON__) || defined(__ARM64_NEON__)

#include "vdrawhelper.h"
#include <arm_neon.h>

extern "C" void pixman_composite_src_n_8888_asm_neon(int32_t w, int32_t h,
                                                     uint32_t *dst,
                                                     int32_t   dst_stride,
                                                     uint32_t  src)
{
    for (int y = 0; y < h; y++) {
        uint32_t *row = dst + y * dst_stride;
        int x = 0;
        for (; x + 4 <= w; x += 4) {
            vst1q_u32(row + x, vdupq_n_u32(src));
        }
        for (; x < w; x++) {
            row[x] = src;
        }
    }
}

extern "C" void pixman_composite_over_n_8888_asm_neon(int32_t w, int32_t h,
                                                      uint32_t *dst,
                                                      int32_t   dst_stride,
                                                      uint32_t  src)
{
    uint32_t sa = (src >> 24) & 0xff;
    uint32_t isa = 255 - sa;
    for (int y = 0; y < h; y++) {
        uint32_t *row = dst + y * dst_stride;
        for (int x = 0; x < w; x++) {
            uint32_t d = row[x];
            uint32_t r = ((src & 0xff) * sa + (d & 0xff) * isa) / 255;
            uint32_t g = (((src >> 8) & 0xff) * sa + ((d >> 8) & 0xff) * isa) / 255;
            uint32_t b = (((src >> 16) & 0xff) * sa + ((d >> 16) & 0xff) * isa) / 255;
            uint32_t a = sa + ((d >> 24) & 0xff) * isa / 255;
            row[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

void memfill32(uint32_t *dest, uint32_t value, int length)
{
    pixman_composite_src_n_8888_asm_neon(length, 1, dest, length, value);
}

void comp_func_solid_SourceOver_neon(uint32_t *dest, int length, uint32_t color,
                                     uint32_t const_alpha)
{
    if (const_alpha != 255) color = BYTE_MUL(color, const_alpha);

    pixman_composite_over_n_8888_asm_neon(length, 1, dest, length, color);
}
#endif
