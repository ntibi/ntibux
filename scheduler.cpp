#include "scheduler.hpp"


class scheduler sched;


void task::init(u32 id, u32 entry, page_directory *pd)
{
    this->id = id;

    this->esp = (u32)mem.kheap.alloc(KERNEL_STACK_SIZE, ALLOC_ALIGNED) + KERNEL_STACK_SIZE; // start at the end of the stack

    memset((u32*)this->esp - sizeof(u32) * 11, 0, sizeof(u32) * 11);
    *(u32*)(this->esp - sizeof(u32) * 1) = (u32)kill_me;
    *(u32*)(this->esp - sizeof(u32) * 2) = entry;
    this->esp -= sizeof(u32) * 11; // flags + 8 regs + eip
    /* new_task stack state:
     * &kill_me()
     * entry_point
     * 9 * 0 for the first popa, popf
     * ...
     */
    this->pd = pd;
    this->elapsed = 0;
    this->created = timer.ticks;
}

void task::init(const char *name, u32 id, u32 entry, page_directory *pd)
{
    init(id, entry, pd);
    set_name(name);
}

void task::set_name(const char *name) { strncpy(this->name, name, TASK_NAME_LEN); }

void task::kill()
{
    this->tasks.del();
}

void scheduler::init()
{
    task *kernel;

    kernel = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    kernel->init("kernel", this->next_id++, 0, mem.kernel_pd);

    this->tasks.init_head();
    this->tasks.push(&kernel->tasks);
#ifdef DEBUG_SCHED
    term.printk(KERN_DEBUG LOG_SCHED "new task %8g%s%g(%u)\n", kernel->name, kernel->id);
#endif
    this->current = kernel;
}

task *scheduler::new_task(const char *name, void (*entry)())
{
    task *new_task;

    disable_interrupts();

    new_task = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    new_task->init(name, this->next_id++, (u32)entry, mem.kernel_pd->clone());

    this->tasks.push_back(&new_task->tasks);

#ifdef DEBUG_SCHED
    term.printk(KERN_DEBUG LOG_SCHED "new task %8g%s%g(%u)\n", new_task->name, new_task->id);
#endif

    enable_interrupts();
    return new_task;
}

task *scheduler::new_task(void (*entry)())
{
    return new_task("", entry);
}

void scheduler::yield()
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

    old->elapsed++;

    context_switch(&old->esp, next->esp);
}

void scheduler::kill_current_task()
{
    u32 trash;

    disable_interrupts();
#ifdef DEBUG_SCHED
    term.printk(KERN_DEBUG LOG_SCHED "task %8g%s%g(%u) killed\n", current->name, current->id);
#endif
    current->kill();
    mem.kheap.free(current, sizeof(task));
    current = LIST_HEAD(this->tasks, tasks, struct task);

    mem.load_page_directory(current->pd);
    mem.switch_page_directory();

    context_switch(&trash, this->current->esp);
}

void kill_me()
{
    sched.kill_current_task();
}

void scheduler::dump()
{
    task *it;

    disable_interrupts();
    LIST_FOREACH_ENTRY(it, &this->tasks, tasks)
    {
        term.printk("%u %s: %U\n", it->id, it->name, it->created);
    }
    enable_interrupts();
}
