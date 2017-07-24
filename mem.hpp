#ifndef MEM_HPP
# define MEM_HPP

#include "multiboot.h"
#include "header.hpp"
#include "kernel.hpp"
#include "list.hpp"

struct page // page table entry
{
    page() : frame(0), flags(0) {}
    page(u32 address, u32 flags) : frame(address >> 12), flags(flags) {}
    u32 frame : 20; // physical page address
    union
    {
        struct
        {
            u8 present : 1; // is present ?
            u8 w : 1;       // writing rights
            u8 user : 1;    // userspace access
            u8 r1 : 2;
            u8 accessed : 1;
            u8 dirty : 1;
            u8 r2 : 2;
            u8 unused : 3;
        };
        u32 flags : 12;
    };
    void claim(u32 frame, u32 kernel, u32 writeable);
    void free(void);
};

struct page_table
{
    page pages[1024];
};

struct page_directory
{
    page_table *tables[1024];
    u32 paddr[1024];
};

class frames
{
public:
    void init(u32 nframes);
    void mark_frame(u32 faddr);
    void unmark_frame(u32 faddr);
    u32 is_free(u32 faddr);
    u32 get_free_frame();
private:
    u32 *frames;
    u32 nframes;
};

#define ALLOC_ALIGNED (1 << 0)
#define ALLOC_ZEROED (1 << 1)
class kheap
{
public:
    kheap();
    void *alloc(u32 size);
    void *alloc(u32 size, u32 flags);
    void init();
private:
    u32 free_zone;
};

class mem
{
public:
    mem();
    void init(u32 high_mem);
    class kheap kheap;
    page *get_page(u32 address, page_directory *pd);
    void switch_page_directory(struct page_directory *pd);
private:
    void alloc_frame(page *p, u32 kernel, u32 writeable);
    void free_frame(page *p);

    class frames frames;

    u32 total;
    page_directory kernel_pd;
};

#endif
