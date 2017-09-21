#include "kernel.hpp"
#include "misc.hpp"

void panic(const char fn[], u32 line, const char msg[])
{
    disable_interrupts();
    term.printk("%4G%0gPANIC%r@%s:%d  %12G%s%G\n", fn, line, msg);
    debug.trace();
    while (1);
}
