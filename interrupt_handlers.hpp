#ifndef INTERRUPT_HANDLERS_HPP
# define INTERRUPT_HANDLERS_HPP

#include "kernel.hpp"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI   0x20

struct int_registers // stack state when handling interrupt
{
    u32 ds; // push eax
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha
    u32 int_nbr;
    union
    {
    u32 err_code;
    u32 pic_int_nbr; // we push real int nbr in pic handler
    };
    u32 eip, cs, eflags, useresp, ss; // pushed by the interrupt
};

extern "C" i32 interrupts_semaphore;
extern "C" void push_ints();
extern "C" void pop_ints();

void dump_int_summary();

extern "C" void interrupt_handler(const int_registers ir);
extern "C" void pic_interrupt_handler(const int_registers ir);

void add_interrupt_handler(u32 nbr, void (*handler)(const int_registers*));
void add_pic_interrupt_handler(u32 nbr, void (*handler)(const int_registers*));


void set_interrupts_handlers();

#define PIC_TIMER 0
#define PIC_KBD 1

#define INT_GPF 13
#define INT_PAGE_FAULT 14

void timer_handler(const int_registers*);
void keyboard_handler(const int_registers*);

void general_protection_fault_handler(const int_registers *ir);
void page_fault_handler(const int_registers *ir);

extern "C" void isr_0(int_registers ir);
extern "C" void isr_1(int_registers ir);
extern "C" void isr_2(int_registers ir);
extern "C" void isr_3(int_registers ir);
extern "C" void isr_4(int_registers ir);
extern "C" void isr_5(int_registers ir);
extern "C" void isr_6(int_registers ir);
extern "C" void isr_7(int_registers ir);
extern "C" void isr_8(int_registers ir);
extern "C" void isr_9(int_registers ir);
extern "C" void isr_10(int_registers ir);
extern "C" void isr_11(int_registers ir);
extern "C" void isr_12(int_registers ir);
extern "C" void isr_13(int_registers ir);
extern "C" void isr_14(int_registers ir);
extern "C" void isr_15(int_registers ir);
extern "C" void isr_16(int_registers ir);
extern "C" void isr_17(int_registers ir);
extern "C" void isr_18(int_registers ir);
extern "C" void isr_19(int_registers ir);
extern "C" void isr_20(int_registers ir);
extern "C" void isr_21(int_registers ir);
extern "C" void isr_22(int_registers ir);
extern "C" void isr_23(int_registers ir);
extern "C" void isr_24(int_registers ir);
extern "C" void isr_25(int_registers ir);
extern "C" void isr_26(int_registers ir);
extern "C" void isr_27(int_registers ir);
extern "C" void isr_28(int_registers ir);
extern "C" void isr_29(int_registers ir);
extern "C" void isr_30(int_registers ir);
extern "C" void isr_31(int_registers ir);

extern "C" void pic_isr_0(int_registers ir);
extern "C" void pic_isr_1(int_registers ir);
extern "C" void pic_isr_2(int_registers ir);
extern "C" void pic_isr_3(int_registers ir);
extern "C" void pic_isr_4(int_registers ir);
extern "C" void pic_isr_5(int_registers ir);
extern "C" void pic_isr_6(int_registers ir);
extern "C" void pic_isr_7(int_registers ir);
extern "C" void pic_isr_8(int_registers ir);
extern "C" void pic_isr_9(int_registers ir);
extern "C" void pic_isr_10(int_registers ir);
extern "C" void pic_isr_11(int_registers ir);
extern "C" void pic_isr_12(int_registers ir);
extern "C" void pic_isr_13(int_registers ir);
extern "C" void pic_isr_14(int_registers ir);
extern "C" void pic_isr_15(int_registers ir);

#endif
