#ifndef MEM_HPP
# define MEM_HPP

#include "multiboot.h"
#include "header.hpp"
#include "kernel.hpp"
#include "list.hpp"

#define PAGE_PRESENT (1 << 0)
#define PAGE_RO (0 << 1)
#define PAGE_RW (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_KERN (0 << 2)
struct page // page table entry
{
    page() : address(0) { }
    page(u32 val) : address(val) { }
    page(u32 address, u32 flags) { this->address = (address & 0xfffff000) & (flags & 0xfff); }
    page& operator=(u32 v) { this->address = v; return *this; }
    void claim(u32 frame, u32 kernel, u32 writeable);
    void free(void);

    u32 address; // paddr(20) | flags(12)
};

struct page_table
{
    page pages[1024];
};

struct page_directory
{
    page_table *tables[1024]; // vaddr
    u32 paddrs[1024]; // paddr | flags
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
    void init(u32 reserve);
    void expand(u32 min);
    void enable_paging(); // call before activating paging
private:
    u32 reserve;
    u32 start;
    u32 free_zone;
    bool paging_enabled;
};

class mem
{
public:
    mem();
    void init(u32 high_mem);
    class kheap kheap;
    page *get_page(u32 address);
    void enable_paging();
    void switch_page_directory(struct page_directory *pd);
    void invalidate_page(u32 page_addr);
    u32 map(u32 vaddr, u32 kernel, u32 writeable);
    u32 map(u32 vaddr, u32 paddr, u32 kernel, u32 writeable);
    void dump();

    page_directory *current_pd;

private:
    void identity_map_kernel();
    u32 alloc_frame(page *p, u32 kernel, u32 writeable);
    u32 alloc_frame(page *p, u32 frame, u32 kernel, u32 writeable);
    void free_frame(page *p);

    u32 get_paddr(u32 vaddr);

    class frames frames;

    u32 total;
    page_directory *kernel_pd;
    bool paging_enabled;
};

#endif
