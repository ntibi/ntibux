#ifndef SCHEDULER_HPP
# define SCHEDULER_HPP

#include "header.hpp"
#include "list.hpp"
#include "mem.hpp"
#include "interrupt_handlers.hpp"
#include "spinlock.hpp"


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
    u32 stack; // stack top
    u32 stack_size;

    union {
    u32 status;
    struct {
    };
    };

    u64 created;
    u64 elapsed;

    struct page_directory *pd;

    char name[TASK_NAME_LEN];

    list tasks;

    void init(u32 id, u32 entry, page_directory *pd);
    void init(const char *name, u32 id, u32 entry, page_directory *pd);
    void set_name(const char *name);
    void kill(); // TODO: free pd
};

class scheduler
{
public:
    void init();
    task *new_task(void (*entry)());
    task *new_task(const char *name, void (*entry)());
    void yield();
    void kill_current_task();
    void dump();
    void dump(u32 id);
    task *get_current() { return current; }

private:
    u32 next_id;

    task *current;

    list tasks;
    spinlock lock;
};

void kill_me();

extern "C" void context_switch(u32 *old_esp, u32 new_esp) __attribute__((fastcall));

#endif
