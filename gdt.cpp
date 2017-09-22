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
#ifdef DEBUG_GDT
    term.printk(KERN_DEBUG LOG_GDT "gate[%d]: base: %x, limit: %x, access: %x, flags: %x\n", n, base, limit, access, flags);
#endif
}

void GDT::init(void)
{
    this->gdt_entries = (struct entry*)GDT_ENTRIES_ADDRESS;
    GDT::flags flags(GDT::flags::GRANULARITY_4KB | GDT::flags::MODE_32B); // operand size: 32b, granularity: 4KB

#ifdef DEBUG_GDT
    term.printk(KERN_DEBUG LOG_GDT "setting GDT entries\n");
#endif
    this->set_gate(0, 0, 0, 0, 0);                                                                                                                                     // NULL segment
    this->set_gate(1, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::CODE | GDT::access::READABLE_CODE  | GDT::access::NON_CONFORMING).raw, flags.raw); // kernel code
    this->set_gate(2, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_UP      ).raw, flags.raw); // kernel data
    this->set_gate(3, 0, 0xFFFFFFFF, GDT::access(GDT::access::KERNEL | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_DOWN    ).raw, flags.raw); // kernel stack
    this->set_gate(4, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::CODE | GDT::access::READABLE_CODE  | GDT::access::NON_CONFORMING).raw, flags.raw); // user code
    this->set_gate(5, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_UP      ).raw, flags.raw); // user data
    this->set_gate(6, 0, 0xFFFFFFFF, GDT::access(GDT::access::USER   | GDT::access::DATA | GDT::access::WRITEABLE_DATA | GDT::access::GROWS_DOWN    ).raw, flags.raw); // user stack
    // this->set_gate(7, (u32)&this->tss, sizeof(this->tss), GDT::access(GDT::access::CODE).raw, flags.raw);
    this->set_gate(7, (u32)&this->tss, sizeof(this->tss), 0x89, flags.raw); // TSS TODO: use real flags
    this->gdt_ptr.size = (sizeof(GDT::entry) * this->gdt_entries_size) - 1;
    this->gdt_ptr.offset = this->gdt_entries;
    this->load_gdt();
}

void GDT::load_gdt(void)
{
    asm volatile (  "lgdt [%0];"
                    "mov ax, 0x10;" // 0x10 <- data selector (gdt[2])
                    "mov ds, ax;"
                    "mov es, ax;"
                    "mov fs, ax;"
                    "mov gs, ax;"
                    "mov ss, ax;"
                    "jmp 0x8:1f;" // 0x8 <- code selector (gdt[1])
                    "1:"
                    :: "r"(&this->gdt_ptr));
#ifdef DEBUG_GDT
    term.printk(KERN_DEBUG LOG_GDT "GDT loaded\n");
#endif
}

void GDT::load_task_register()
{
    asm volatile ("ltr ax" :: "a" (&this->tss));
}
