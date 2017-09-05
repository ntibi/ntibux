#include "mem.hpp"

extern u32 _kbeginning;
extern u32 _kend;

u32 kbeginning = (u32)&_kbeginning;
u32 kend = (u32)&_kend;

static inline u32 off(u32 b) { return b % 32; }
static inline u32 index(u32 b) { return b / 32; }

void page::claim(u32 frame, u32 kernel, u32 writeable)
{
    this->address = (frame & 0xfffff000) |
                    PAGE_PRESENT |
                    (kernel ? PAGE_KERN : PAGE_USER) |
                    (writeable ? PAGE_RW : PAGE_RO);
}

void page::free(void)
{
    if (!this->address)
    {
        term.printk(KERN_ERROR LOG_MM "freeing non allocated frame (0x%x)\n", this->address & 0xfffff000);
        return ;
    }
    this->address = 0;
}


class mem mem;

mem::mem() : kheap(), total(0), paging_enabled(false) { }

void mem::init(u32 high_mem)
{
    this->total = high_mem << 10;
    term.printk(KERN_INFO LOG_MM "detected mem: %uMB (%u pages)\n", this->total >> 20, (this->total) / PAGESIZE);

    this->kheap.init(PAGESIZE * 64);
    this->frames.init(this->total / PAGESIZE);

    this->identity_map_kernel();
    this->kheap.enable_paging();

    this->switch_page_directory(this->kernel_pd);
    this->enable_paging();
    this->paging_enabled = true;
}

void mem::identity_map_kernel()
{
    this->kernel_pd = (page_directory*)this->kheap.unpaged_alloc(sizeof(page_directory), ALLOC_ALIGNED | ALLOC_ZEROED);
    this->current_pd = this->kernel_pd;

    this->map_range(0, 0, kend, 1, 0); // identity mapping kernel code
#ifdef DEBUG_MM
    term.printk(KERN_DEBUG LOG_MM "identity mapped the %d first frames (0x%x - 0x%x)\n", ((kend + 0xfff) & 0xfffff000) / PAGESIZE, 0, (kend + 0xfff) & 0xfffff000);
#endif
}

u32 mem::map(u32 vaddr, u32 kernel, u32 writeable)
{
    vaddr &= 0xfffff000;
    return this->alloc_frame(this->get_page(vaddr), kernel, writeable);
}

u32 mem::map(u32 vaddr, u32 paddr, u32 kernel, u32 writeable)
{
    vaddr &= 0xfffff000;
    paddr &= 0xfffff000;
    return this->alloc_frame(this->get_page(vaddr), paddr, kernel, writeable);
}

u32 mem::map_range(u32 vaddr, u32 range, u32 kernel, u32 writeable)
{
    u32 i = 0;

    vaddr &= 0xfffff000;
    while (i < range)
    {
        if (!this->map(vaddr + i, kernel, writeable))
            PANIC("can't map requested memory");
        i += PAGESIZE;
    }
    return 1;
}

u32 mem::map_range(u32 vaddr, u32 paddr, u32 range, u32 kernel, u32 writeable)
{
    u32 i = 0;

    vaddr &= 0xfffff000;
    paddr &= 0xfffff000;
    while (i < range)
    {
        if (!this->map(vaddr + i, paddr + i, kernel, writeable))
            PANIC("can't map requested memory");
        i += PAGESIZE;
    }
    return 1;
}


page *mem::get_page(u32 address)
{
    u32 directory_index;
    u32 table_index;
    // adddress    : xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // dir index   : 1111111111                      
    // table index :           1111111111            
    // offset      :                     111111111111

    directory_index = (address >> 22) & 0x3ff;
    table_index = (address >> 12) & 0x3ff;
    if (!this->current_pd->tables[directory_index])
    {
        page_table *pt;
        if (this->paging_enabled)
        {
            pt = (page_table*)this->kheap.alloc(sizeof(page_table), ALLOC_ALIGNED | ALLOC_ZEROED);
            this->current_pd->tables[directory_index] = pt;
            this->current_pd->paddrs[directory_index] = (u32)this->get_paddr((u32)pt) | 0x7;
        }
        else
        {
            pt = (page_table*)this->kheap.unpaged_alloc(sizeof(page_table), ALLOC_ALIGNED | ALLOC_ZEROED);
            this->current_pd->tables[directory_index] = pt;
            this->current_pd->paddrs[directory_index] = (u32)pt | 0x7;
        }
    }
    return &this->current_pd->tables[directory_index]->pages[table_index];
}

void mem::enable_paging()
{
#ifdef DEBUG_MM
    term.printk(KERN_DEBUG LOG_MM "enabling paging\n");
#endif
    asm volatile (
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            );
}

void mem::switch_page_directory(struct page_directory *pd)
{
    this->current_pd = pd;
#ifdef DEBUG_MM
    term.printk(KERN_DEBUG LOG_MM "switching page directory\n");
#endif
    asm volatile ("mov cr3, %0;" :: "r"(pd->paddrs));
}

void mem::invalidate_page(u32 page_addr)
{
    asm volatile ("invlpg [%0]" :: "r"(page_addr));
}

u32 mem::get_paddr(u32 vaddr)
{
    return (this->current_pd->tables[(vaddr >> 22) & 0x3ff]->pages[(vaddr >> 12) & 0x3ff].address & ~0xfff) + (vaddr & 0xfff);
}

void mem::status()
{
    this->frames.status();
}

void mem::dump()
{
    u32 start_addr, prev, current;
    bool dump = false;

    prev = 0;
    start_addr = 0;
    for (u32 addr = 0; addr < 1024U * 1024U * 4096U - 4096; addr += PAGESIZE)
    {
        if (this->current_pd->tables[(addr >> 22) & 0x3ff] && this->current_pd->tables[(addr >> 22) & 0x3ff]->pages[(addr >> 12) & 0x3ff].address)
            current = this->current_pd->tables[(addr >> 22) & 0x3ff]->pages[(addr >> 12) & 0x3ff].address;
        else
            current = 0;

        if (current)
        {
            if (!prev)
                start_addr = addr;
            else if ((current & 0x7) != (prev & 0x7)) // fin de pages contigues ayant les memes flags
                dump = true;
        }
        else if (prev) // fin de pages contigues
        {
            dump = true;
        }

        if (dump)
        {
            term.printk("0x%8x - 0x%8x %8x %cr%c\n", start_addr, addr,
                    addr - start_addr,
                    (prev & PAGE_USER) ? 'u' : '-',
                    (prev & PAGE_RW) ? 'w' : '-');
            dump = false;
            start_addr = addr;
        }
        prev = current;
    }
}

void frames::init(u32 nframes)
{
    this->nframes = nframes;
    this->frames = (u32*)mem.kheap.unpaged_alloc(sizeof(u32) * index(this->nframes), ALLOC_ZEROED);
}

void frames::mark_frame(u32 faddr)
{
    faddr >>= 12;
    this->frames[index(faddr)] |= (1 << off(faddr));
}

void frames::unmark_frame(u32 faddr)
{
    faddr >>= 12;
    this->frames[index(faddr)] &= ~(1 << off(faddr));
}

u32 frames::is_free(u32 faddr)
{
    faddr >>= 12;
    return !(this->frames[index(faddr)] & (1 << off(faddr)));
}

u32 frames::get_free_frame()
{
    u32 i, j;

    for (i = 0; i < index(this->nframes); ++i)
    {
        if (this->frames[i] == 0xffffffff)
            continue;
        for (j = 0; j < 32; ++j)
        {
            if (!(frames[i] & (1 << j)))
                return (i * 32 + j) << 12;
        }
    }
    return 0;
}

void frames::status()
{
    u32 nbr = 0;
    u32 tmp;
    u32 i;

    for (i = 0; i < index(this->nframes); ++i)
    {
        for (tmp = frames[i]; tmp; tmp >>=1, ++nbr);
    }
    term.printk("physical memory usage:\n");
    term.printk("  %d/%d pages (%d%%)\n", nbr, this->nframes, nbr * 100 / this->nframes);
}

u32 mem::alloc_frame(page *p, u32 kernel, u32 writeable)
{
    u32 frame;

    frame = this->frames.get_free_frame();
    if (!frame)
        PANIC("no available frame found");
    return this->alloc_frame(p, frame, kernel, writeable);
}

u32 mem::alloc_frame(page *p, u32 frame, u32 kernel, u32 writeable)
{
    if (!this->frames.is_free(frame))
    {
        term.printk(KERN_ERROR LOG_MM "frame 0x%x already mapped to a page\n", frame);
        return 0;
    }
    this->frames.mark_frame(frame);
    p->claim(frame, kernel, writeable);
    return 1;
}

void mem::free_frame(page *p)
{
    this->frames.unmark_frame(p->address & 0xfffff000);
    p->free();
}


kheap::kheap() : paging_enabled(false), reserve_order(0), kheap_start(0), free_zone(0) { }

void kheap::init(u32 reserve)
{
    this->min_order = 12; // (1 << 12) = PAGESIZE
    this->max_order = this->min_order;
    while ((1U << this->max_order) < mem.total) ++this->max_order;
    this->max_order = this->max_order - 3; // 1/8 of the total memory

    this->orders = max_order - min_order + 1;
    this->min_alloc = 1 << min_order; // PAGESIZE
    this->max_alloc = 1 << max_order;

    reserve = (reserve + 0xfff) & ~0xfff; // reserve must be at least a page
    while ((1U << this->reserve_order) < reserve) ++this->reserve_order; // get reserve order
    if (this->reserve_order < this->min_order)
    {
#ifdef DEBUG_KHEAP
        term.printk(KERN_WARNING LOG_KHEAP "kernel heap reserve is too small (%d), scaled up to (%d)\n", this->reserve_order, this->min_order);
#endif
        this->reserve_order = this->min_order;
    }
    if (this->reserve_order > this->max_order)
    {
#ifdef DEBUG_KHEAP
        term.printk(KERN_WARNING LOG_KHEAP "kernel heap reserve is too big (%d), scaled down to (%d)\n", this->reserve_order, this->max_order);
#endif
        this->reserve_order = this->max_order;
    }

#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "reserve size: 0x%x(order %d<%d<%d)\n", 1U << this->reserve_order, this->min_order, this->reserve_order, this->max_order);
#endif

    this->kheap_start = (kend + 0xfff) & ~0xfff;
    this->free_zone = this->kheap_start;
}

void kheap::enable_paging()
{
    u32 new_free_zone = (this->free_zone + ((1 << this->max_order) - 1)) & ~((1 << this->max_order) - 1); // max_order last bits must be 0 at the beginning

    this->free_blocks = (u32*)this->unpaged_alloc(sizeof(u32) * this->orders, ALLOC_ZEROED);

    this->paging_enabled = true;

    this->free_zone = (this->free_zone + 0xfff) & ~0xfff;

#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "identity mapping %p -> %p\n", this->kheap_start, this->free_zone + PAGESIZE);
#endif

    //                                                                                      __________/ needed for the mapping of the reserve
    mem.map_range(this->kheap_start, this->kheap_start, this->free_zone - this->kheap_start + PAGESIZE, 1, 1); // identity map already used memory

#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "classic mapping %p -> %p\n", new_free_zone, new_free_zone + (1 << this->reserve_order));
#endif
    mem.map_range(new_free_zone, 1U << this->reserve_order, 1, 1); // classic map for the reserve

    this->free_zone = new_free_zone;

    this->get_free_block(this->reserve_order) = this->free_zone;
    *(u32*)this->get_free_block(this->reserve_order) = 0;
}

void kheap::double_reserve()
{
    if (this->reserve_order >= this->max_order)
    {
        term.printk(KERN_WARNING LOG_KHEAP "cannot expand heap reserve anymore\n");
        return ;
    }
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "expanding reserve order %d->%d (0x%x->0x%x)\n", this->reserve_order, this->reserve_order + 1, 1U << this->reserve_order, 1U << (this->reserve_order + 1));
#endif
    mem.map_range(this->free_zone + (1U << this->reserve_order), 1U << this->reserve_order, 1, 1);
    *(u32*)(this->free_zone + (1U << this->reserve_order)) = this->get_free_block(this->reserve_order) ? *(u32*)this->get_free_block(this->reserve_order) : 0;
    this->get_free_block(this->reserve_order) = this->free_zone + (1U << this->reserve_order);
    this->reserve_order++;
}

void *kheap::buddy_alloc(u32 order)
{
    void *out = 0;

    if (!this->get_free_block(order))
    {
        while (order >= this->reserve_order)
        {
            if (order > this->max_order)
                PANIC("too big alloc for buddy");
            this->double_reserve();
        }
    }

    if (this->get_free_block(order)) // free block found
    {
        out = (void*)this->get_free_block(order);
        this->get_free_block(order) = *(u32*)this->get_free_block(order);
    }
    else // split an higher order block
    {
        void *buddy;

        out = this->buddy_alloc(order + 1);
        buddy = (void*)((u32)out ^ (1U << order)); // get buddy of out
        *(u32*)buddy = this->get_free_block(order);
        this->get_free_block(order) = (u32)buddy; // push unused split (buddy) in the front of the free_blocks list
    }
    return out;
}

void kheap::buddy_free(void *addr, u32 order)
{
    void *buddy;
    void *tmp;

    buddy = (void*)((u32)addr ^ (1U << order));
    tmp = &this->get_free_block(order);
    while (*(u32*)tmp && *(void**)tmp != buddy)
        tmp = *(void**)tmp;
    if (!*(void**)tmp) // buddy is alloced
    {
        *(u32*)addr = this->get_free_block(order);
        this->get_free_block(order) = (u32)addr; // push addr in front of free list
    }
    else // buddy is free too, recursive merge
    {
        *(void**)tmp = *(void**)buddy; // remove buddy from free list
        this->buddy_free((void*)((u32)addr & ~(1U << order)), order + 1);
    }
}


inline u32 kheap::get_order(u32 size)
{
    u32 order = 0;

    while ((1U << order) < size) ++order;
    if (order < this->min_order)
        order = this->min_order;
    return order;
}

void *kheap::alloc(u32 size) { return this->alloc(size, 0); }

void *kheap::alloc(u32 size, u32 flags)
{
    void *out;
    u32 order = this->get_order(size);

    out = this->buddy_alloc(order);

    if (flags & ALLOC_ZEROED)
        memset(out, 0, size);
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "alloc %p(s:%u(%u), p:%c%c)\n", out, size, 1 << order, flags & ALLOC_ALIGNED ? 'A' : ' ', flags & ALLOC_ZEROED ? '0' : ' ');
#endif
    return out;
}

void kheap::free(void *addr, u32 size)
{
    u32 order = this->get_order(size);
    this->buddy_free(addr, order);
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "free  %p(s:%u(%u))\n", addr, size, 1 << order);
#endif
}

void *kheap::unpaged_alloc(u32 size) { return this->unpaged_alloc(size, 0); }

void *kheap::unpaged_alloc(u32 size, u32 flags)
{
    void *out;

    if (flags & ALLOC_ALIGNED)
        this->free_zone = (this->free_zone + 0xfff) & ~0xfff;
    out = (void*)this->free_zone;
    this->free_zone += size;
    if (flags & ALLOC_ZEROED)
        memset(out, 0, size);
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG LOG_KHEAP "ualloc %p(s:%u, f:%c%c)\n", out, size, flags & ALLOC_ALIGNED ? 'A' : ' ', flags & ALLOC_ZEROED ? '0' : ' ');
#endif
    return out;
}
