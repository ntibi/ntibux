#include "terminal.hpp"

terminal term;

unsigned char terminal::kbdus[128] = {
    0,    27,  '1', '2', '3',  '4', '5', '6', '7',  '8', /* 9 */
    '9',  '0', '-', '=', '\b',                           /* Backspace */
    '\t',                                                /* Tab */
    'q',  'w', 'e', 'r',                                 /* 19 */
    't',  'y', 'u', 'i', 'o',  'p', '[', ']', '\n',      /* Enter key */
    0,                                                   /* 29   - Control */
    'a',  's', 'd', 'f', 'g',  'h', 'j', 'k', 'l',  ';', /* 39 */
    '\'', '`', 0,                                        /* Left shift */
    '\\', 'z', 'x', 'c', 'v',  'b', 'n',                 /* 49 */
    'm',  ',', '.', '/', 0,                              /* Right shift */
    '*',  0,                                             /* Alt */
    ' ',                                                 /* Space bar */
    0,                                                   /* Caps lock */
    0,                                                   /* 59 - F1 key ... > */
    0,    0,   0,   0,   0,    0,   0,   0,   0,         /* < ... F10 */
    0,                                                   /* 69 - Num lock*/
    0,                                                   /* Scroll Lock */
    0,                                                   /* Home key */
    0,                                                   /* Up Arrow */
    0,                                                   /* Page Up */
    '-',  0,                                             /* Left Arrow */
    0,    0,                                             /* Right Arrow */
    '+',  0,                                             /* 79 - End key*/
    0,                                                   /* Down Arrow */
    0,                                                   /* Page Down */
    0,                                                   /* Insert Key */
    0,                                                   /* Delete Key */
    0,    0,   0,   0,                                   /* F11 Key */
    0,                                                   /* F12 Key */
    0, /* All other keys are undefined */
};

unsigned char terminal::uppercase[128] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, '"', 0, 0, 0, 0, '<', '_', '>', '?',
')', '!', '@', '#', '$', '%', '^', '&', '*', '(', 0, ':', 0, '+', 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
'{', '|', '}', 0, 0, '~',
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
0, 0, 0, 0,
};

char terminal::klog[8][20] = {
    "%4G%0g[EMRG]%r ",
    "%4g[ALT]%r ",
    "%12g[CRI]%r ",
    "%12g[ERR]%r ",
    "%14g[WAR]%r ",
    "%11g[NOT]%r ",
    "%6g[INF]%r ",
    "%3g[DBG]%r ",
};

terminal::terminal() :
    x(0), y(0),
    color(0), def_color(0), cursor(' ' | ((0 | (6 << 4)) << 8)),
    shift(0), ctrl(0), alt(0),
    vga_buffer(0),
    sx(0), sy(0),
    log_level(7)
{ }

void terminal::init()
{
#ifdef USE_HW_CURSOR
    this->enable_cursor();
#else
    this->disable_cursor();
#endif
}

int terminal::set_log_level(u8 new_ll)
{
    if (new_ll <= 7)
    {
        this->log_level = new_ll;
        return 1;
    }
    return 0;
}
u8 terminal::get_log_level() { return this->log_level; }

void terminal::set_vga_buffer(u16 *vga_buffer) { this->vga_buffer = vga_buffer; }
void terminal::set_default_color(u8 def_color) { this->def_color = def_color; }

void terminal::save_pos(void)
{
    this->sx = this->x;
    this->sy = this->y;
}

void terminal::load_pos(void)
{
    this->x = this->sx;
    this->y = this->sy;
}

void terminal::set_pos(size_t x, size_t y)
{
	this->x = x;
	this->y = y;
}


char terminal::get_scancode(void)
{
    u8 c;

wait_key:
    while (!(inb(0x64) & 1));
    c = inb(0x60);
    switch (c)
    {
        case 0x1d: // ctrl
        case 0x9d: // release
            this->ctrl = !!!(c & 0x80);
            goto wait_key;
            break;
        case 0x2a: // shift
        case 0xaa: // release
            this->shift = !!!(c & 0x80);
            goto wait_key;
            break;
        case 0x38: // alt
        case 0xb8: // release
            this->alt = !!!(c & 0x80);
            goto wait_key;
            break;
    }
    if (c & 0x80)
        goto wait_key;
#ifdef DEBUG_GETSCANCODE
    this->save_pos();
    this->set_pos(10, 0);
    printk_noup("|%c%c%c|", this->ctrl ? 'C' : ' ', this->shift ? 'S' : ' ', this->alt ? 'A' : ' ');
    this->load_pos();
#endif
    return c;
}

keypress terminal::getchar(void)
{
    keypress k;

    this->update_cursor();
    k.scancode = get_scancode();
    if (this->shift && uppercase[kbdus[k.scancode]])
        k.asc = uppercase[kbdus[k.scancode]];
    else
        k.asc = kbdus[k.scancode];
    k.ctrl = this->ctrl;
    k.alt = this->alt;
#ifdef DEBUG_GETCHAR
    this->save_pos();
    this->set_pos(0, 0);
    printk("|%x %x %c|", kbdus[k.scancode], k.asc, is_sep(k.asc) ? ' ' : k.asc);
    this->load_pos();
#endif
    return k;
}

void terminal::update_vga_buffer(void)
{
    size_t dst;

    for (dst = 0; dst < VGA_WIDTH * VGA_HEIGHT; ++dst)
        this->vga_buffer[dst] = this->cache_buffer[dst];
}

void terminal::tcur_scroll(void)
{
    u16 newbuf[VGA_WIDTH * VGA_HEIGHT];
    u16 v = vga_entry(' ', this->def_color);
    size_t dst, src;

    for (dst = 0, src = VGA_WIDTH; src < VGA_WIDTH * VGA_HEIGHT; ++dst, ++src)
    {
        newbuf[dst] = this->cache_buffer[src];
    }
    while (dst < VGA_WIDTH * VGA_HEIGHT)
        newbuf[dst++] = v;
    for (dst = 0; dst < VGA_WIDTH * VGA_HEIGHT; ++dst)
        this->cache_buffer[dst] = newbuf[dst];
}

void terminal::tcur_nextl(void)
{
	this->x = 0;
    if (this->y < VGA_HEIGHT - 1)
        ++this->y;
    else
        tcur_scroll();
}

void terminal::tcur_next(void)
{
	if (this->x < VGA_WIDTH - 1)
	{
		++this->x;
	}
	else
	{
		this->x = 0;
		tcur_nextl();
	}
}

void terminal::clear(void)
{
    u16 c = vga_entry(' ', this->def_color);

    this->x = 0;
    this->y = 0;
    for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
    {
        this->cache_buffer[i] = c;
    }
    this->update_vga_buffer();
}

void terminal::set_fg(enum vga_color fg)
{
    this->color = fg | (this->color & 0xf0);
}

void terminal::set_bg(enum vga_color bg)
{
    this->color = (this->color & 0xf) | bg << 4;
}

void terminal::reset_color(void)
{
	this->color = this->def_color;
}

void terminal::set_color(u8 color)
{
	this->color = color;
}

void terminal::set_color(enum vga_color fg, enum vga_color bg)
{
	this->color = vga_entry_color(fg, bg);
}

void terminal::tmov(size_t x, size_t y)
{
	this->x = x;
	this->y = y;
}

void terminal::tputc_xy(char c, size_t x, size_t y)
{
    this->tputc_xy_noup(c, x, y);
    this->update_vga_buffer();
}

void terminal::tputc_xy_noup(char c, size_t x, size_t y)
{
	this->x = x;
	this->y = y;
	this->cache_buffer[y * VGA_WIDTH + x] = vga_entry(c, this->color);
}

void terminal::tputc(char c)
{
    this->tputc_noup(c);
    this->update_vga_buffer();
}

void terminal::tputc_noup(char c)
{
    if (c == '\n')
    {
        tcur_nextl();
    }
    else
    {
        this->cache_buffer[this->y * VGA_WIDTH + this->x] = vga_entry(c, this->color);
        tcur_next();
    }
}

size_t terminal::tputs(const char *s, size_t n)
{
    size_t ret;

    ret = this->tputs_noup(s, n);
    this->update_vga_buffer();
    return ret;
}

size_t terminal::tputs(const char *s)
{
    size_t ret;

    ret = this->tputs_noup(s);
    this->update_vga_buffer();
    return ret;
}

size_t terminal::tputs_noup(const char *s, size_t n)
{
    size_t i = n;
	while (i--)
	{
        tputc_noup(*s);
		++s;
	}
    return n;
}

size_t terminal::tputs_noup(const char *s)
{
    char *p = (char*)s;

	while (*p)
	{
        tputc_noup(*p);
		++p;
	}
    return p - s;
}

int terminal::check_binds(keypress k)
{
    if (k.ctrl)
    {
        switch (k.asc)
        {
        case 'l':
            this->clear();
            return 1;
        }
    }
    return 0;
}

size_t terminal::tread_line(char *buf, size_t n)
{
    size_t i = 0;
    keypress k;

    while (i < n - 1)
    {
#ifdef DEBUG_READLINE
        this->save_pos();
        this->set_pos(35, 0);
        printk("|%u|", i);
        this->load_pos();
#endif
        k = getchar();
        if (check_binds(k))
            continue;
        if (k.asc == '\t')
            k.asc = ' ';
        if (k.asc == '\b')
        {
            if (i > 0)
            {
                i--;
                if (this->x > 0)
                {
                    this->x--;
                }
                else if (this->y > 0)
                {
                    this->x = VGA_WIDTH - 1;
                    this->y--;
                } // else {x = 0; y = 0}
            tputc_xy(' ', this->x, this->y);
            }
            continue;
        }
        buf[i] = k.asc;
        ++i;
        tputc(k.asc);
        if (k.asc == '\n')
            return i;
    }
    return i;
}

void terminal::disable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x3f);
}

void terminal::enable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x1f);
}

void terminal::update_cursor(void)
{
#ifdef USE_HW_CURSOR
    u16 pos = (this->y * VGA_WIDTH) + this->x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
#else
    this->vga_buffer[y * VGA_WIDTH + x] = this->cursor;
#endif
}

void terminal::set_cursor(u16 new_cursor)
{
    this->cursor = new_cursor;
}
