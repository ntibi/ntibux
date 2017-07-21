#include "mem.hpp"

class mem mem;

mem::mem() : kheap(), total(0), pages(0) { }

void mem::init(u32 high_mem)
{
    this->kheap.init();
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

extern u32 end;

kheap::kheap() : free_zone(0) { }

void kheap::init()
{
    this->free_zone = (u32)&end;
}

void *kheap::alloc(u32 size)
{
    void *out = (void*)this->free_zone;
    this->free_zone += size;
    return out;
}
