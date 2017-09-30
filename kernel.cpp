#include "kernel.hpp"

void motd()
{
    term.printk("%11g\n");

    term.printk("                                                                              \n");
    term.printk("                            _   _ _                                           \n");
    term.printk("                      _ __ | |_(_) |__  _   ___  __                           \n");
    term.printk("                     | '_ \\| __| | '_ \\| | | \\ \\/ /                       \n");
    term.printk("                     | | | | |_| | |_) | |_| |>  <                            \n");
    term.printk("                     |_| |_|\\__|_|_.__/ \\__,_/_/\\_\\                       \n");

    term.printk("%g\n");
}


static void shell()
{
    interpreter interp;
    char buf[MAX_LINE_LEN];
    size_t len;
    int ret;

    while (1)
    {
        term.tputs("> ");
        len = term.tread_line(buf, MAX_LINE_LEN);
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
    term.init();
    term.set_vga_buffer((u16*)0xb8000);
    term.set_default_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    term.set_cursor(vga_entry(' ', vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY)));
    term.clear();
    term.set_log_level(LL_DEBUG);

    motd();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        LOG(KERN_CRIT "multiboot magic not matching... (%p)\n", magic);
        return ;
    }
    if (mboot->cmdline && *(char*)mboot->cmdline)
        LOG(KERN_INFO "booting %9g%s%r\n", mboot->cmdline);
    else
        LOG(KERN_WARNING "no cmdline\n");

    debug.init(mboot);
    gdt.init();
    mem.init(mboot->mem_upper);
    idt.init();
    timer.init();
    sched.init();
    set_interrupts_handlers();

    push_ints();

    shell();
}
