#include "timer.hpp"

class timer timer;

void timer::tick()
{
#ifdef DEBUG_TIMER
    term.save_pos();
    term.tputc_xy(this->ticks % 2 ? ' ' : 'T', 79, 0);
    term.load_pos();
    term.update_cursor();
#endif
    this->ticks++;
    return ;
}

void timer::set_freq(u32 hz)
{
    u32 div = CLOCK_BASE_FREQ / hz;

    term.printk(KERN_INFO LOG_TIMER "setting timer frequency at %uHz (%u ms interval)\n", hz, hz / 1000);
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
    term.printk("elapsed time %u seconds (%U ticks)\n", this->ticks / CLOCK_FREQ, this->ticks);
}
