#include "syscalls.hpp"

void kill_me()
{
    sched.kill_current_task();
}

void sleep(u64 ms)
{
    sched.get_current()->add_sleep(max(ms / MS_INTERVAL, TIME_SLICE));
    sched.yield();
}
