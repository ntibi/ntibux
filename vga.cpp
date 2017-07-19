#include "vga.hpp"

u8 vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

u16 vga_entry(unsigned char uc, u8 color)
{
    return (u16)uc | (u16)color << 8;
}


