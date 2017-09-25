#include "spinlock.hpp"

spinlock::spinlock() : locked(0)
{
}

void spinlock::lock()
{
    asm volatile (
            "1:" // spin
            "test [%0], 1;"
            "jnz 1b;"
            "lock bts %0, 0;" // sets CF at the old value of the set bit
            "jc 1b;"
            :: "m"(locked));
}

void spinlock::release()
{
    asm volatile (
            "mov [%0], 0;"
            :: "m"(locked));
}
