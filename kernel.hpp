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
#include "timer.hpp"
#include "scheduler.hpp"
#include "spinlock.hpp"


extern class terminal term;
extern class GDT gdt;
extern class mem mem;
extern class IDT idt;
extern class timer timer;
extern class scheduler sched;
extern class debug debug;

extern u32 _kbeginning;
extern u32 _kend;
extern u32 _stack_top;
extern u32 _stack_bottom;

void motd();

#endif
