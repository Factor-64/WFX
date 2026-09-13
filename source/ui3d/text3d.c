#include "text3d.h"
#include "font.h"
#include "global.h"

void draw_char(u8* backbuf, int x, int y, char c, u8 color)
{
    if(c < 16 || c > 191) return;

    int idx = c - 16;
    int gx = idx & 15;
    int gy = idx >> 4;

    const u8* bmp = (const u8*)fontBitmap;

    int bytes_per_row = 14;
    int glyph_offset = gy * (bytes_per_row * 9);

    const u8* glyph = bmp + glyph_offset;

    u8* dst = backbuf + (MUL240(y)) + x;

    for(int row = 0; row < 9; ++row)
    {
        int bit_offset = gx * 7;
        int byte_offset = bit_offset >> 3;
        int bit_shift  = bit_offset & 7;

        const u8* rowptr = glyph + row * bytes_per_row;

        u8 raw  = rowptr[byte_offset];
        u8 next = rowptr[byte_offset + 1];

        u8 bits = (raw >> bit_shift) | (next << (8 - bit_shift));
        bits &= 0x7F;

        for(int col = 0; col < 7; col++)
        {
            if(bits & (1 << col))
                dst[col] = color;
        }

        dst += 240;
    }
}

void draw_text(u8* backbuf, int x, int y, const char *str, u8 color)
{
    int start_x = x;

    while(*str)
    {
        if(*str == '\n')
        {
            y += 11;
            x = start_x;
            ++str;
            continue;
        }

        draw_char(backbuf, x, y, *str, color);
        x += 7;
        ++str;
    }
}
