#ifndef SCHEDULER_HPP
# define SCHEDULER_HPP

#include "header.hpp"
#include "list.hpp"
#include "mem.hpp"
#include "interrupt_handlers.hpp"


struct regs
{
    u32 eax, ebx, ecx, edx;
    u32 esi, edi;
    u32 esp, ebp;
    u32 eip;
    u32 eflags;
} __attribute__((packed));

class task
{
public:
    u32 id;
    u32 esp;

    struct page_directory *pd;

    list tasks;

    void kill();
};

class scheduler
{ // TODO replace sti/cli by more complete locks
public:
    void init();
    task *new_task(void (*entry)());
    void yield();
    void kill_current_task();

private:
    u32 next_id = 1;

    task *current = NULL;

    list tasks;
};

void kill_me();

extern "C" void context_switch(u32 *old_esp, u32 new_esp) __attribute__((fastcall));

#endif
