#include "kernel.hpp"
#include "misc.hpp"

void trace()
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
