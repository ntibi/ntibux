#include "idt.hpp"

IDT idt;

void IDT::init()
{
    this->idt.base = (u32)&this->entries;
    this->idt.limit = sizeof(idt_entry) * IDT_ENTRIES - 1;

    this->set_gate(0,  isr_0,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(1,  isr_1,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(2,  isr_2,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(3,  isr_3,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(4,  isr_4,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(5,  isr_5,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(6,  isr_6,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(7,  isr_7,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(8,  isr_8,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(9,  isr_9,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(10, isr_10, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(11, isr_11, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(12, isr_12, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(13, isr_13, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(14, isr_14, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(15, isr_15, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(16, isr_16, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(17, isr_17, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(18, isr_18, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(19, isr_19, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(20, isr_20, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(21, isr_21, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(22, isr_22, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(23, isr_23, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(24, isr_24, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(25, isr_25, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(26, isr_26, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(27, isr_27, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(28, isr_28, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(29, isr_29, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(30, isr_30, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(31, isr_31, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);

    outb(PIC1_CMD, 0x11); // remap pic isrs [0-15] to [32-47]
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
    this->set_gate(32, pic_isr_0,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(33, pic_isr_1,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(34, pic_isr_2,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(35, pic_isr_3,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(36, pic_isr_4,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(37, pic_isr_5,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(38, pic_isr_6,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(39, pic_isr_7,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(40, pic_isr_8,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(41, pic_isr_9,  SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(42, pic_isr_10, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(43, pic_isr_11, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(44, pic_isr_12, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(45, pic_isr_13, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(46, pic_isr_14, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);
    this->set_gate(47, pic_isr_15, SS_KCODE, IDT_PRESENT | IDT_GATE32 | IDT_DEFAULT | IDT_ALL);

    add_pic_interrupt_handler(PIC_KBD, keyboard_handler);

    this->load_idt();
}

void IDT::set_gate(u8 n, void (*handler)(struct int_registers), u16 ss, u8 flags)
{
    this->entries[n].base_low = (u32)handler & 0xffff;
    this->entries[n].base_high = ((u32)handler >> 16) & 0xffff;

    this->entries[n].ss = ss;

    this->entries[n].zero = 0;

    this->entries[n].flags = flags;
}

void IDT::load_idt()
{
    asm volatile ("lidt [%0]" :: "r"((u32)&this->idt));
#ifdef DEBUG_IDT
    term.printk(KERN_DEBUG LOG_IDT "IDT loaded\n");
#endif
}
