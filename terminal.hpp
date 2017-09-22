#ifndef TERMINAL_HPP
# define TERMINAL_HPP

#include "vga.hpp"
#include "header.hpp"
#include "misc.hpp"

class keypress
{
public:
    u8 asc;
    u8 scancode;
    u8 ctrl : 1;
    u8 alt : 1;
};

class kbd_buf // keyboard events buffer
{
public:
    kbd_buf() : index(0) {};
    void put(u8 c) { if (index < KEYBOARD_BUFFER_SIZE - 1) buffer[index++] = c; };
    u8 get() { return index ? buffer[--index] : 0; };
    u8 available() { return !!index; }

private:
    u32 index;
    u8 buffer[KEYBOARD_BUFFER_SIZE];
};

class terminal
{
public:
    terminal();
    void init();

    void set_vga_buffer(u16*);
    void set_default_color(u8);

    void clear(void);

    void save_pos(void);
    void load_pos(void);
    void set_pos(size_t x, size_t y);

    // colors
    void set_color(enum vga_color fg, enum vga_color bg);
    void set_fg(enum vga_color fg);
    void set_bg(enum vga_color bg);
    void set_color(u8 color);
    void reset_color(void);

    // cursor
    void disable_cursor(void);
    void enable_cursor(void);
    void update_cursor(void);
    void set_cursor(u16 new_cursor);

    // borders
    void tcur_beginning(void);
    void tcur_next(void);
    void tcur_nextl(void);
    void tcur_scroll(void);

    // input
    keypress getchar(void);
    void kbd_ready_callback();

    // output
    void tputc_xy(char c, size_t x, size_t y);
    void tputc_xy_noup(char c, size_t x, size_t y);
    void tputc(char c);
    void tputc_noup(char c);
    size_t tputs(const char *s, size_t n);
    size_t tputs(const char *s);
    size_t tputs_noup(const char *s, size_t n);
    size_t tputs_noup(const char *s);
    size_t tread_line(char *buf, size_t n);
    void tmov(size_t x, size_t y);

    // printk
    size_t printk(const char *format, ...);
    size_t printk_noup(const char *format, ...);

    u8 get_log_level();
    int set_log_level(u8 new_ll);

private:
    size_t x;
    size_t y;
    u8 color;
    u8 def_color;
    u16 cursor;

    kbd_buf kb;

    u8 shift : 1;
    u8 ctrl : 1;
    u8 alt : 1;

    volatile u16 *vga_buffer;
    u16 cache_buffer[VGA_HEIGHT * VGA_WIDTH];
    size_t sx;
    size_t sy;
    static unsigned char kbdus[128];
    static unsigned char uppercase[128];

    u8 log_level;
    static char klog[8][20];

    size_t vprintk(const char *format, va_list args);

    char get_scancode(void);
    void update_vga_buffer(void);

    template <typename T>
    size_t putuint_b(T nbr, unsigned int base, int padding);  // NO VGA UPDATE
    size_t putint_b(int nbr, unsigned int base, int padding); // !!!

    int check_binds(keypress k);
};

#endif
