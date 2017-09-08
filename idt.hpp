#ifndef IDT_HPP
# define IDT_HPP

#include "kernel.hpp"
#include "interrupt_handlers.hpp"


#define SS_KCODE 0x8 // kernel code is gdt[1]

#define IDT_PRESENT (1 << 7)
#define IDT_GATE32 (1 << 3)
#define IDT_DEFAULT ((1 << 2) | (1 << 1)) // ???
#define IDT_ALL (3 << 6) // ring 3
#define IDT_KERNEL (0 << 6) // ring 0
struct idt_entry
{
    idt_entry() : base_low(0), ss(0), zero(0), flags(0), base_high(0) {};
    u16 base_low;
    u16 ss;
    u8 zero;
    u8 flags;
    u16 base_high;
} __attribute__((packed));

struct idt_header
{
    u16 limit;
    u32 base;
} __attribute__((packed));

class IDT
{
public:
    void init();
private:
    void set_gate(u8 n, void (*handler) (struct int_registers), u16 ss, u8 flags);
    void load_idt();
    idt_header idt;
    idt_entry entries[IDT_ENTRIES] = {};
};

#endif
