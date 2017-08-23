#include "kernel.hpp"
#include "misc.hpp"

void panic(char *fn, u32 line, char *msg)
{
    term.printk("%4G%0gPANIC%r@%s:%d  %12G%s%G\n", fn, line, msg);
    debug.trace();
    while (1);
}

