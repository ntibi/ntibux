#include "debug.hpp"

class debug debug;

debug::debug() : elf_sec(0) { }

void debug::init(struct multiboot_info *mboot)
{
    if (mboot->flags & (1 << 5) && mboot->u.elf_sec.num)
    {
        this->elf_sec = &mboot->u.elf_sec;
        term.printk(KERN_NOTICE "debug informations found (%d symbols)\n", this->elf_sec->num);
    }
    else
    {
        term.printk(KERN_NOTICE "no debug informations found\n");
    }
}

void debug::trace()
{
    u32 *stack;
    size_t frame = 0;
    u32 ebp, eip;

    asm volatile ( "mov %0, ebp" : "=a"(stack) );
    while (stack && stack < &stack_top)
    {
        ebp = stack[0];
        eip = stack[1];
        term.printk("%u: %8x in ??? ()\n", frame, eip);
        stack = (u32*)ebp;
        ++frame;
    }
}
