#include "scheduler.hpp"


class scheduler sched;


void scheduler::init()
{
    task *kernel;

    kernel = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    kernel->id = this->next_id++;
    // kernel->regs.* will be set on first task_switching
    kernel->pd = mem.kernel_pd;

    this->tasks.init_head();
    this->tasks.push(&kernel->tasks);
#ifdef DEBUG_SCHED
    term.printk(KERN_DEBUG LOG_SCHED "main task %d\n", kernel->id);
#endif
    this->current = kernel;
}

task *scheduler::new_task(void (*entry)())
{
    task *new_task;

    disable_interrupts();

    new_task = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    new_task->id = this->next_id++;

    new_task->esp = (u32)mem.kheap.alloc(KERNEL_STACK_SIZE, ALLOC_ALIGNED) + KERNEL_STACK_SIZE; // start at the end of the stack
    memset((u32*)new_task->esp - sizeof(u32) * 10, 0, sizeof(u32) * 10);
    *(u32*)(new_task->esp - sizeof(u32)) = (u32)entry;
    new_task->esp -= sizeof(u32) * 10; // flags + 8 regs + eip

    new_task->pd = mem.kernel_pd->clone();

    this->tasks.push_back(&new_task->tasks);

#ifdef DEBUG_SCHED
    term.printk(KERN_DEBUG LOG_SCHED "new task %d\n", new_task->id);
#endif

    enable_interrupts();
    return new_task;
}

void scheduler::yield(int_registers const *ir)
{
    task *old;
    task *next;

    if (!this->current || this->tasks.singular())
        return ;

    disable_interrupts();

    old = LIST_HEAD(this->tasks, tasks, struct task);
    this->tasks.rotate();
    next = LIST_HEAD(this->tasks, tasks, struct task);

    mem.load_page_directory(next->pd);
    mem.switch_page_directory();

    this->current = next;

    context_switch(&old->esp, next->esp);
}
