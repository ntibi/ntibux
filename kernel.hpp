#ifndef KERNEL_HPP
# define KERNEL_HPP

#include "header.hpp"
#include "vga.hpp"
#include "gdt.hpp"
#include "terminal.hpp"
#include "interpreter.hpp"
#include "multiboot.h"
#include "mem.hpp"
#include "misc.hpp"
#include "debug.hpp"
#include "idt.hpp"


extern class terminal term;
extern class GDT gdt;
extern class mem mem;
extern class IDT idt;
extern class debug debug;

extern u32 stack_top;
extern u32 stack_bottom;

void motd();

#endif
