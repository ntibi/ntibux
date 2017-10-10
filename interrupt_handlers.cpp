#include "interrupt_handlers.hpp"

u32 interrupts_counter[256] = {0};
u32 pic_interrupts_counter[16] = {0};

void (*int_handler[32])(const int_registers*);
void (*pic_int_handler[16])(const int_registers*);

// interrupts: disabled <= 0 < enabled
i32 interrupts_semaphore = 0; // used by (push|pop)_ints

void dump_int_summary()
{
    for (u32 i = 0; i < sizeof(pic_interrupts_counter) / sizeof(u32); ++i)
    {
        if (pic_interrupts_counter[i])
            term.printk("PIC int %x: %d\n", i, pic_interrupts_counter[i]);
    }
    for (u32 i = 0; i < sizeof(interrupts_counter) / sizeof(u32); ++i)
    {
        if (interrupts_counter[i])
            term.printk("int %x: %d\n", i, interrupts_counter[i]);
    }
}

void interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    LOG(KERN_DEBUG "int %d: 0x%x \n", int_regs.int_nbr, int_regs.err_code);
#endif

    interrupts_counter[int_regs.int_nbr]++;

    if (int_handler[int_regs.int_nbr])
    {
        int_handler[int_regs.int_nbr](&int_regs);
    }
    else
    {
        term.printk("unknown interrupt (%d)\n", int_regs.int_nbr);
        PANIC("unhandled interrupt\n");
    }
}

void pic_interrupt_handler(const int_registers int_regs)
{
#ifdef DEBUG_INTERRUPTS
    if (int_regs.pic_int_nbr != 0 && int_regs.pic_int_nbr != 1) // skip: timer, kbd
        LOG(KERN_DEBUG "PIC int %d\n", int_regs.pic_int_nbr);
#endif

    if (int_regs.pic_int_nbr >= 8)
        outb(PIC2_CMD, PIC_EOI);

    outb(PIC1_CMD, PIC_EOI);

    pic_interrupts_counter[int_regs.pic_int_nbr]++;

    if (pic_int_handler[int_regs.pic_int_nbr])
    {
        pic_int_handler[int_regs.pic_int_nbr](&int_regs);
    }
    else
    {
        term.printk("unknown interrupt (%d)\n", int_regs.pic_int_nbr);
        PANIC("unhandled PIC interrupt\n");
    }
}

void add_interrupt_handler(u32 nbr, void (*handler)(const int_registers*))
{
    int_handler[nbr] = handler;
}

void add_pic_interrupt_handler(u32 nbr, void (*handler)(const int_registers*))
{
    pic_int_handler[nbr] = handler;
}


void set_interrupts_handlers()
{
    add_interrupt_handler(INT_GPF            , general_protection_fault_handler);
    add_interrupt_handler(INT_ZERO_DIV       , divide_by_zero_handler);
    add_interrupt_handler(INT_DBG            , debug_trap_handler);
    add_interrupt_handler(INT_NMI            , nmi_handler);
    add_interrupt_handler(INT_BREAKPOINT     , breakpoint_handler);
    add_interrupt_handler(INT_OVERFLOW       , overflow_handler);
    add_interrupt_handler(INT_BOUND          , bound_check_fail_handler);
    add_interrupt_handler(INT_OPCODE         , invalid_opcode_handler);
    add_interrupt_handler(INT_FEATURE        , non_available_feature_handler);
    add_interrupt_handler(INT_DOUBLE         , double_fault_handler);
    add_interrupt_handler(INT_TSS            , invalid_tss_handler);
    add_interrupt_handler(INT_SEGMENT        , invalid_segment_handler);
    add_interrupt_handler(INT_STACK          , invalid_stack_segment_handler);
    add_interrupt_handler(INT_X87            , fpu_floating_point_exception_handler);
    add_interrupt_handler(INT_ALIGNMENT      , alignment_check_handler);
    add_interrupt_handler(INT_SIMD           , simd_floating_point_exception_handler);
    add_interrupt_handler(INT_VIRTUALIZATION , virtualization_exception_handler);
}


void timer_handler(const int_registers *ir)
{
    (void)ir;
    timer.tick();
    sched.get_current()->tick();

    if (!(timer.ticks % TIME_SLICE))
    {
        interrupts_semaphore++; // if yield doesnt return (task switched) we won't have the push_ints from interrupt_handler
        sched.yield();
        interrupts_semaphore--; // if yield returns we get back to regular count (we will have the push_ints from interrupt_handlers)
    }
}

void keyboard_handler(const int_registers *ir)
{
    (void)ir;
    term.kbd_ready_callback();
}

void divide_by_zero_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "division by zero @eip: 0x%x\n", ir->eip);
    kill_me();
}

void debug_trap_handler(const int_registers *ir)
{
    (void)ir;
    LOG(KERN_INFO "debug interruption");
}

void nmi_handler(const int_registers *ir)
{
    (void)ir;
    PANIC("unknown hardware failure");
}

void breakpoint_handler(const int_registers *ir)
{
    LOG(KERN_INFO "breakpoint hit @eip: 0x%x\n", ir->eip);
}

void overflow_handler(const int_registers *ir)
{
    (void)ir;
    LOG(KERN_WARNING "overflow\n");
    kill_me();
}

void bound_check_fail_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "bound check failed @eip: 0x%x\n", ir->eip);
    kill_me();
}

void invalid_opcode_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "invalid opcode @eip: 0x%x\n", ir->eip);
    kill_me();
}

void non_available_feature_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "use of non supported/activated cpu feature @eip: 0x%x\n", ir->eip);
    kill_me();
}

void double_fault_handler(const int_registers *ir)
{
    (void)ir;
    PANIC("double fault");
}

void invalid_tss_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "invalid task switch segment selector @eip: 0x%x\n", ir->eip);
}

void invalid_segment_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "invalid segment selector 0x%x @eip: 0x%x\n", ir->err_code, ir->eip);
}

void invalid_stack_segment_handler(const int_registers *ir)
{
    LOG(KERN_WARNING "invalid stack segment selector 0x%x @eip: 0x%x\n", ir->err_code, ir->eip);
}

void general_protection_fault_handler(const int_registers *ir)
{
    LOG(KERN_EMERG "general protection fault @eip: 0x%x\n", ir->eip);
    kill_me();
}

void page_fault_handler(const int_registers *ir)
{
    u32 fault;

    asm volatile ("mov %0, cr2" : "=r"(fault)); // fault addr is in cr2

    LOG(KERN_EMERG "page fault @0x%x  %c%c%c%c%c\n", fault,
            ir->err_code & 0x1  ? 'v' : 'p', // protection violation or non present page
            ir->err_code & 0x2  ? 'w' : 'r', // fault caused by read or write
            ir->err_code & 0x4  ? 'u' : 'k', // user or kernel
            ir->err_code & 0x8  ? 'c' : '-', // accessed cpu bits of page (PSE or PAE enabled)
            ir->err_code & 0x10 ? 'i' : '-'  // caused by instruction (NX enabled)
            );
    kill_me();
}

void fpu_floating_point_exception_handler(const int_registers *ir)
{
    LOG(KERN_EMERG "floating point exception @eip: 0x%x\n", ir->eip);
    // TODO: check detailed error in x87 regs
    kill_me();
}

void alignment_check_handler(const int_registers *ir)
{
    LOG(KERN_EMERG "ailgnment check failed @eip: 0x%x\n", ir->eip);
    kill_me();
}

void simd_floating_point_exception_handler(const int_registers *ir)
{
    LOG(KERN_EMERG "SIMD floating point exception @eip: 0x%x\n", ir->eip);
    // TODO: check detailed error in SSE reg MXCSR
    kill_me();
}

void virtualization_exception_handler(const int_registers *ir)
{
    LOG(KERN_EMERG "virtualization exception @eip: 0x%x\n", ir->eip);
    kill_me();
}
