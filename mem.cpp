#include "mem.hpp"

class mem mem;

mem::mem() : total(0), pages(0) { }

void mem::init(u32 high_mem)
{
    this->total = high_mem * 1024;
    this->pages = this->total / PAGESIZE;
    term.printk(KERN_INFO "detected mem: %uMB (%u pages)\n", this->total >> 20, this->pages);
}

void mem::enable_paging()
{
    asm volatile (
            "mov cr3, %0;"
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            :: "r"(0));
}
