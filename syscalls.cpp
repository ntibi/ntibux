#include "syscalls.hpp"

void kill_me()
{
    sched.kill_current_task();
}

void sleep(u64 ms)
{
    sched.get_current()->add_sleep(ms * MS_INTERVAL);
    sched.yield();
}
