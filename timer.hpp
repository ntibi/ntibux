#ifndef TIMER_HPP
# define TIMER_HPP

#include "kernel.hpp"

#define CLOCK_BASE_FREQ 1193180U

#define PIT_DATA1 0x40
#define PIT_DATA2 0x41
#define PIT_DATA3 0x42
#define PIT_COMMAND 0x43


class timer
{
public:
    timer() : ticks(0) { };
    void tick();
    void set_freq(u32 hz);
    void init();
    void dump();

    static u64 msecs(u64 ticks) { return ticks * 1000 / CLOCK_FREQ; }
    static u64 secs(u64 ticks)  { return ticks / CLOCK_FREQ; }
    static u64 mins(u64 ticks)  { return ticks / CLOCK_FREQ / 60; }
    static u64 hours(u64 ticks) { return ticks / CLOCK_FREQ / 60 / 60; }
    static u64 days(u64 ticks)  { return ticks / CLOCK_FREQ / 60 / 60 / 24; }

    u64 ticks;
};

#endif
