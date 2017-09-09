#include "interrupt_handlers.hpp"

u32 interrupts_counter[256] = {0};
u32 pic_interrupts_counter[16] = {0};

void (*pic_int_handler[16])();

void enable_interrupts()
{
#ifdef DEBUG_INTERRUPTS
    term.printk(KERN_DEBUG "enabling interrupts\n");
#endif
    asm volatile ("sti");
}

void disable_interrupts()
{
#ifdef DEBUG_INTERRUPTS
    term.printk(KERN_DEBUG "disabling interrupts\n");
#endif
    asm volatile ("cli");
}

void dump_int_summary()
{
    for (u32 i = 0; i < sizeof(pic_interrupts_counter) / sizeof(u32); ++i)
    {
        if (pic_interrupts_counter[i])
            term.printk("PIC int %x: %d\n", i, pic_interrupts_counter[i]);
    }
    for (u32 i = 0; i < sizeof(interrupts_counter) / sizeof(u32); ++i)
    {
        if (interrupts_counter[i])
            term.printk("int %x: %d\n", i, interrupts_counter[i]);
    }
}

void interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    term.printk(KERN_DEBUG "int %d: 0x%x \n", int_regs.int_nbr, int_regs.err_code);
#endif

    interrupts_counter[int_regs.int_nbr]++;
}

void pic_interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    if (int_regs.pic_int_nbr != 0 && int_regs.pic_int_nbr != 1) // skip: timer, kbd
        term.printk(KERN_DEBUG "PIC int %d\n", int_regs.pic_int_nbr);
#endif

    if (pic_int_handler[int_regs.pic_int_nbr])
        pic_int_handler[int_regs.pic_int_nbr]();

    if (int_regs.pic_int_nbr >= 8)
        outb(PIC2_CMD, PIC_EOI);

    outb(PIC1_CMD, PIC_EOI);

    pic_interrupts_counter[int_regs.pic_int_nbr]++;
}

void add_interrupt_handler(u32 nbr, void (*handler)())
{
    pic_int_handler[nbr] = handler;
}
