#include "kernel.hpp"

static void shell(terminal &term)
{
    interpreter interp(term);
    char buf[MAX_LINE_LEN];
    size_t len;
    int ret;

    while (1)
    {
        term.tputs("> ");
        len = term.tread_line(buf, sizeof(buf));
        buf[len] = 0;
        ret = interp.interpret(buf);
        if (ret)
        {
            /* do smth */
        }
    }
}

extern "C" void kernel_main(struct multiboot_info *mboot, u32 magic)
{
    GDT gdt;
    mem mem;

    term.init();
    term.set_vga_buffer((u16*)0xb8000);
    term.set_default_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    term.set_cursor(vga_entry(' ', vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY)));
    term.clear();
    term.set_log_level(LL_DEBUG);

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        term.printk(KERN_CRIT "multiboot magic not matching... (%p)\n", magic);
        return ;
    }
    if (mboot->cmdline && *(char*)mboot->cmdline)
        term.printk(KERN_INFO "booting %9g%s%r\n", mboot->cmdline);
    else
        term.printk(KERN_WARNING "no cmdline\n");

    gdt.init();
    mem.init(mboot->mem_upper);

    shell(term);
}
