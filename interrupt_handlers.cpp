#include "interrupt_handlers.hpp"

void (*pic_irq_handler[16])();

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

void interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    term.printk(KERN_DEBUG "int %d: 0x%x \n", int_regs.int_nbr, int_regs.err_code);
#endif
}

void pic_interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    if (int_regs.pic_int_nbr != 0 && int_regs.pic_int_nbr != 1) // skip: timer, kbd
        term.printk(KERN_DEBUG "PIC int %d\n", int_regs.pic_int_nbr);
#endif

    if (pic_irq_handler[int_regs.pic_int_nbr])
        pic_irq_handler[int_regs.pic_int_nbr]();

    if (int_regs.pic_int_nbr >= 8)
        outb(PIC2_CMD, PIC_EOI);

    outb(PIC1_CMD, PIC_EOI);
}

