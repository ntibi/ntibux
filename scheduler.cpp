#include "scheduler.hpp"


class scheduler sched;

u32 stack_top = (u32)&_stack_top;
u32 stack_bottom = (u32)&_stack_bottom;

void task::init(u32 id, u32 entry, page_directory *pd)
{
    this->id = id;

    this->stack = (u32)mem.kheap.alloc(KERNEL_STACK_SIZE, ALLOC_ALIGNED) + KERNEL_STACK_SIZE; // start at the end of the stack
    this->stack_size = KERNEL_STACK_SIZE;

    memset((u32*)this->stack - sizeof(u32) * 11, 0, sizeof(u32) * 11);
    *(u32*)(this->stack - sizeof(u32) * 1) = (u32)kill_me;
    *(u32*)(this->stack - sizeof(u32) * 2) = entry;
    this->esp = this->stack - (sizeof(u32) * 11); // flags + 8 regs + eip
    /* new_task stack state:
     * &kill_me()
     * entry_point
     * 9 * 0 for the first popa, popf
     * ...
     */
    this->pd = pd;
    this->elapsed = 0;
    this->created = timer.ticks;
    this->status = 0;
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

    lock.lock();

    this->next_id = 0;

    // manually creating initial kernel task
    kernel = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    kernel->set_name("kernel");
    kernel->id = this->next_id++;
    kernel->stack = stack_top;
    kernel->stack_size = stack_top - stack_bottom;
    kernel->esp = 0; // will be set when switched out
    kernel->pd = mem.kernel_pd;
    kernel->elapsed = 0;
    kernel->created = timer.ticks;
    kernel->status = 0;

    this->tasks.init_head();
    this->tasks.push(&kernel->tasks);
#ifdef DEBUG_SCHED
    LOG(KERN_DEBUG LOG_SCHED "new task %8g%s%g(%u)\n", kernel->name, kernel->id);
#endif
    this->current = kernel;
    lock.release();
}

task *scheduler::new_task(const char *name, void (*entry)())
{
    task *new_task;

    pop_ints();

    new_task = (task*)mem.kheap.alloc(sizeof(task), ALLOC_ZEROED);
    new_task->init(name, this->next_id++, (u32)entry, mem.kernel_pd);

    lock.lock();
    this->tasks.push_back(&new_task->tasks);
    lock.release();

    push_ints();

#ifdef DEBUG_SCHED
    LOG(KERN_DEBUG LOG_SCHED "new task %8g%s%g(%u)\n", new_task->name, new_task->id);
#endif

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

    this->current->elapsed++;

    pop_ints();
    lock.lock();

    if (!this->current || this->tasks.singular())
        goto leave;

    old = LIST_HEAD(this->tasks, tasks, struct task);
    this->tasks.rotate();
    next = LIST_HEAD(this->tasks, tasks, struct task);

    lock.release();

    mem.load_page_directory(next->pd);
    mem.switch_page_directory();

    this->current = next;

#ifdef DEBUG_SCHED_SWITCH
    LOG(KERN_DEBUG LOG_SCHED "%8g%s%g -> %8g%s%g\n", old->name, next->name);
#endif
    context_switch(&old->esp, next->esp);
    return ;
leave:
    lock.release();
    push_ints();
    return ;
}

void scheduler::kill_task(u32 id)
{
    task *it;

    if (id == 0)
        PANIC("killed kernel");

    pop_ints();
    lock.lock();

    if (id == this->current->id)
        this->kill_current_task_locked();

    LIST_FOREACH_ENTRY(it, &this->tasks, tasks)
    {
        if (it->id == id)
        {
#ifdef DEBUG_SCHED
            LOG(KERN_DEBUG LOG_SCHED "task %8g%s%g(%u) killed (after %U ms)\n", it->name, it->id, timer::msecs(it->elapsed));
#endif
            it->kill();
            mem.kheap.free(it, sizeof(task));
        }
    }

    lock.release();
    push_ints();
}

void scheduler::kill_current_task()
{
    pop_ints();
    lock.lock();
    this->kill_current_task_locked();
}

void scheduler::kill_current_task_locked()
{
    u32 trash;

    if (!current->id) // cant kill init task
        PANIC("killed kernel");

#ifdef DEBUG_SCHED
    LOG(KERN_DEBUG LOG_SCHED "task %8g%s%g(%u) killed and switched out (after %U ms)\n", current->name, current->id, timer::msecs(current->elapsed));
#endif
    current->kill();
    mem.kheap.free(current, sizeof(task));
    current = LIST_HEAD(this->tasks, tasks, struct task);
    lock.release();

    mem.load_page_directory(current->pd);
    mem.switch_page_directory();

    context_switch(&trash, this->current->esp);
    return ;
}

void kill_me()
{
    sched.kill_current_task();
}

void scheduler::dump(u32 id)
{
    task *it;

    pop_ints();
    lock.lock();
    LIST_FOREACH_ENTRY(it, &this->tasks, tasks)
    {
        if (it->id == id)
        {
            term.printk("%8g%s%g(%u)\n", it->name, it->id);
            term.printk("stack: 0x%x <- 0x%x <- 0x%x (%u%%)\n", it->stack - it->stack_size, it->esp, it->stack, (it->stack - it->esp) * 100 / it->stack_size);
            term.printk("pd: 0x%x\n", it->pd);
            term.printk("created: %U (%U ms ago)\n", it->created, timer::msecs(timer.ticks - it->created));
            term.printk("elapsed: %U (%U ms)\n", it->elapsed, timer::msecs(it->elapsed));
            break ;
        }
    }
    lock.release();
    push_ints();
}

void scheduler::dump()
{
    task *it;

    pop_ints();
    lock.lock();
    LIST_FOREACH_ENTRY(it, &this->tasks, tasks)
    {
        term.printk("%u: %8g%s%g (%U ms)\n", it->id, it->name, timer::msecs(timer.ticks - it->created));
    }
    lock.release();
    push_ints();
}
