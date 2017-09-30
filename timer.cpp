#include "timer.hpp"

class timer timer;

void timer::tick()
{
    this->ticks++;
    return ;
}

void timer::set_freq(u32 hz)
{
    u32 div = CLOCK_BASE_FREQ / hz;

    LOG(KERN_INFO LOG_TIMER "setting timer frequency at %uHz (%u ms interval)\n", hz, 1000 / hz);
    outb(PIT_COMMAND, 0x36);
    outb(PIT_DATA1, div & 0xff);
    outb(PIT_DATA1, (div >> 8) & 0xff);
}

void timer::init()
{
    set_freq(CLOCK_FREQ);
    add_pic_interrupt_handler(PIC_TIMER, timer_handler);
}

void timer::dump()
{
    term.printk("elapsed time %U seconds (%U ticks)\n", this->ticks / CLOCK_FREQ, this->ticks);
}
