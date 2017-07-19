#include "debug.hpp"

class debug debug;

typedef struct __attribute__((packed)) {
    uint32_t length;
    uint16_t version;
    uint32_t header_length;
    uint8_t min_instruction_length;
    uint8_t default_is_stmt;
    int8_t line_base;
    uint8_t line_range;
    uint8_t opcode_base;
    uint8_t std_opcode_lengths[12];
} debug_line_header;

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
