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
    void claim(u32 frame, u32 flags);
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
    void status();
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
    void *unpaged_alloc(u32 size);
    void *unpaged_alloc(u32 size, u32 flags);

    void *alloc(u32 size);
    void *alloc(u32 size, u32 flags);

    void free(void *addr, u32 size);

    void init(u32 reserve);
    void double_reserve(); // double reserve

    void enable_paging(); // call before activating paging

private:

    u32 min_order;
    u32 max_order;

    u32 orders;
    u32 min_alloc;
    u32 max_alloc;

                         // PRE PAGING | POST PAGING
    bool paging_enabled; // 0          | 1
    u32 reserve_order;   // unused     | memory pool size order
    u32 kheap_start;     // pool start | unpaged pool start
    u32 free_zone;       // pool end   | pool start

    u32 *free_blocks;
    inline u32 &get_free_block(u32 order) { return free_blocks[order - min_order]; }

    inline u32 get_order(u32 size);
    void *buddy_alloc(u32 order);
    void buddy_free(void *addr, u32 order);
};

#define MAP_RO          (0 << 1)
#define MAP_RW          (1 << 1)
#define MAP_KERN        (0 << 2)
#define MAP_USER        (1 << 2)
#define MAP_KERNEL_CODE (MAP_KERN | MAP_RO)
#define MAP_KERNEL_DATA (MAP_KERN | MAP_RW)
#define MAP_USER_CODE   (MAP_USER | MAP_RO)
#define MAP_USER_DATA   (MAP_USER | MAP_RW)
class mem
{ // TODO: write and use TLB flushing functions (invlpg instruction)
public:
    mem();
    void init(u32 high_mem);
    class kheap kheap;
    page *get_page(u32 address);
    void enable_paging();
    void load_page_directory(struct page_directory *pd); // change pd used by mapping functions
    void switch_page_directory();                        // switch cr3 loaded pd by the loaded pd
    void invalidate_page(u32 page_addr);
    u32 map(u32 vaddr, u32 flags);
    u32 map(u32 vaddr, u32 paddr, u32 flags);
    u32 map_range(u32 vaddr, u32 range, u32 flags);
    u32 map_range(u32 vaddr, u32 paddr, u32 range, u32 flags);
    u32 unmap(u32 vaddr);
    u32 unmap_range(u32 vaddr, u32 range);
    void status();
    void dump();

    page_directory *current_pd;

    u32 total;

private:
    void identity_map_kernel();
    u32 alloc_frame(page *p, u32 flags);
    u32 alloc_frame(page *p, u32 frame, u32 flags);
    void free_frame(page *p);

    u32 get_paddr(u32 vaddr);

    class frames frames;

    page_directory *kernel_pd;
    bool paging_enabled;
};

#endif
