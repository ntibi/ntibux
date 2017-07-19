#ifndef MEM_HPP
# define MEM_HPP

#include "multiboot.h"
#include "header.hpp"
#include "kernel.hpp"
#include "list.hpp"

struct page // page table entry
{
    page(u32 address, u32 flags) : paddr(address >> 12), flags(flags) {}
    u32 paddr: 20; // physical page address
    union
    {
        struct
        {
            u8 present : 1;
            u8 w : 1;
            u8 user : 1;
            u8 write_through : 1;
            u8 cache_disable : 1;
            u8 cpu_accessed : 1; // cpu set
            u8 dirty : 1;
            u8 zero : 1;
            u8 global : 1;
            u8 unused : 3;
        };
        u32 flags : 12;
    };
};

struct page_directory // page directory entry
{
    page_directory(u32 address, u32 flags) : addr(address), flags(flags) {}
    u32 addr: 20; // page table address
    union
    {
        struct
        {
            u8 present : 1;
            u8 w : 1;
            u8 user : 1;
            u8 write_through : 1;
            u8 cache_disable : 1;
            u8 cpu_accessed : 1; // cpu set
            u8 zero : 1;
            u8 big_page : 1; // 4MB page (requires PSE)
            u8 ignored : 1;
            u8 unused : 3;
        };
        u32 flags : 12;
    };
};

class mem
{
public:
    mem();
    void init(u32 high_mem);
private:
    void enable_paging();
    u32 total;
    u32 pages;
};

#endif
