#include "gdt.hpp"

GDT gdt;

GDT::GDT() : gdt_entries_size(0) { }

void GDT::set_gate(u8 n, u32 base, u32 limit, u8 access, u8 flags)
{
    this->gdt_entries[n].low_base = (base & 0xFFFF);
    this->gdt_entries[n].mid_base = (base >> 16) & 0xFF;
    this->gdt_entries[n].high_base = (base >> 24) & 0xFF;

    this->gdt_entries[n].low_limit = (limit & 0xFFFF);
    this->gdt_entries[n].high_limit = (limit >> 16) & 0xf;

    this->gdt_entries[n].flags = flags & 0xf;
    this->gdt_entries[n].access = access;

    this->gdt_entries_size = n + 1;
}

void GDT::init(void)
{
    this->gdt_entries = (struct entry*)GDT_ENTRIES_ADDRESS;
    GDT::flags flags(GDT::flags::GRANULARITY_4KB | GDT::flags::MODE_32B); // operand size: 32b, granularity: 4KB

    term.printk(KERN_DEBUG "setting GDT entries\n");
    this->set_gate(0, 0, 0, 0, 0);                                                                                                                                     // NULL segment
    this->set_gate(1, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::CODE | GDT::access::READABLE_CODE  | GDT::access::NON_CONFORMING).raw, flags.raw); // kernel code
    this->set_gate(2, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_UP      ).raw, flags.raw); // kernel data
    this->set_gate(3, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_DOWN    ).raw, flags.raw); // kernel stack
    this->set_gate(4, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::CODE | GDT::access::READABLE_CODE  | GDT::access::NON_CONFORMING).raw, flags.raw); // user code
    this->set_gate(5, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_UP      ).raw, flags.raw); // user data
    this->set_gate(6, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_DOWN    ).raw, flags.raw); // user stack
    this->gdt_ptr.size = (sizeof(GDT::entry) * this->gdt_entries_size) - 1;
    this->gdt_ptr.offset = this->gdt_entries;
    this->load_gdt();
}

void GDT::load_gdt(void)
{
    asm volatile (  "lgdt [%0];"
                    "mov ax, 0x10;" // 0x10 <- data selector
                    "mov ds, ax;"
                    "mov es, ax;"
                    "mov fs, ax;"
                    "mov gs, ax;"
                    "mov ss, ax;"
                    "jmp 0x8:flush;" // 0x8 <- code selector
                    "flush:"
                    :: "r"(&this->gdt_ptr));
    term.printk(KERN_INFO "GDT loaded\n");
}
