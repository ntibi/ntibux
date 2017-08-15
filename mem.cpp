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
        term.printk(KERN_ERROR "freeing non allocated frame (0x%x)\n", this->address & 0xfffff000);
        return ;
    }
    this->address = 0;
}


class mem mem;

mem::mem() : kheap(), total(0), paging_enabled(false) { }

void mem::init(u32 high_mem)
{
    this->total = high_mem * 1024;
    term.printk(KERN_INFO "%5gmm%r: detected mem: %uMB\n", this->total >> 20);

    this->kheap.init(PAGESIZE * 128);
    this->frames.init(this->total / PAGESIZE);

    this->identity_map_kernel();
    this->kheap.enable_paging();

    term.printk(KERN_DEBUG "%5gmm%r: activating paging\n");
    this->switch_page_directory(this->kernel_pd);
    this->paging_enabled = true;
}

void mem::identity_map_kernel()
{
    this->kernel_pd = (page_directory*)this->kheap.alloc(sizeof(page_directory), ALLOC_ALIGNED | ALLOC_ZEROED);
    this->current_pd = this->kernel_pd;

    for (u32 i = 0; i < kend; i += PAGESIZE) // kernel identity mapping
    {
        this->map(i, i, 1, 1);
    }
    term.printk(KERN_INFO "%5gmm%r: identity mapped the %d first frames (0x%x - 0x%x)\n", ((kend + 0xfff) & 0xfffff000) / PAGESIZE, 0, ((kend + 0xfff) & 0xfffff000) - 1);
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

page *mem::get_page_no_create(u32 address)
{
    u32 directory_index;
    u32 table_index;

    directory_index = (address >> 22) & 0x3ff;
    table_index = (address >> 12) & 0x3ff;
    if (!this->current_pd->tables[directory_index])
        return 0;
    return &this->current_pd->tables[directory_index]->pages[table_index];
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
        page_table *pt = (page_table*)this->kheap.alloc(sizeof(page_table), ALLOC_ALIGNED | ALLOC_ZEROED);
        this->current_pd->tables[directory_index] = pt;
        if (this->paging_enabled)
            this->current_pd->paddrs[directory_index] = (u32)this->get_paddr((u32)pt) | 0x7;
        else
            this->current_pd->paddrs[directory_index] = (u32)pt | 0x7;
    }
    return &this->current_pd->tables[directory_index]->pages[table_index];
}

void mem::switch_page_directory(struct page_directory *pd)
{
    this->current_pd = pd;
    asm volatile (
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            :: "r"(pd->paddrs));
}

u32 mem::get_paddr(u32 vaddr)
{
    page *p;

    p = this->get_page_no_create(vaddr);
    return (p->address << 12) + (vaddr & 0xfff);
}

void frames::init(u32 nframes)
{
    this->nframes = nframes;
    this->frames = (u32*)mem.kheap.alloc(sizeof(u32) * index(this->nframes), ALLOC_ZEROED);
}

void frames::mark_frame(u32 faddr)
{
    faddr >>= 12;
    this->frames[index(faddr)] |= (0x1 << off(faddr));
}

void frames::unmark_frame(u32 faddr)
{
    faddr >>= 12;
    this->frames[index(faddr)] &= ~(0x1 << off(faddr));
}

u32 frames::is_free(u32 faddr)
{
    faddr >>= 12;
    return !(this->frames[index(faddr)] & (0x1 << off(faddr)));
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
            if (!(frames[i] & (0x1 << j)))
                return (i*32 + j) << 12;
        }
    }
    return 0;
}

u32 mem::alloc_frame(page *p, u32 kernel, u32 writeable)
{
    u32 frame;

    frame = this->frames.get_free_frame();
    return this->alloc_frame(p, frame, kernel, writeable);
}

u32 mem::alloc_frame(page *p, u32 frame, u32 kernel, u32 writeable)
{
    if (!this->frames.is_free(frame))
    {
        term.printk(KERN_ERROR "%5gmm%r:frame 0x%x already mapped to a page\n", frame);
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


kheap::kheap() : reserve(0), start(0), free_zone(0), paging_enabled(false) { }

void kheap::init(u32 reserve)
{
    this->reserve = (reserve + 0xfff) & ~0xfff;
    this->start = (kend + 0xfff) & ~0xfff;
    this->free_zone = this->start;
}

void kheap::enable_paging()
{
    this->paging_enabled = true;
    if (this->free_zone - this->start > this->reserve)
        this->reserve = this->free_zone - this->start;
    u32 i = this->start;
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG "%5gkheap%r: identity mapping %p -> %p\n", i, this->free_zone);
#endif
    for (; i < this->free_zone; i += PAGESIZE) // identity map the already used mem
        mem.map(i, i, 1, 1);
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG "%5gkheap%r: classic  mapping %p -> %p\n", i, this->start + this->reserve);
#endif
    for (; i < this->start + this->reserve; i += PAGESIZE) // classic map for the rest
        mem.map(i, 1, 1);
}

void kheap::expand(u32 min)
{
    u32 increase;

    if (!this->paging_enabled)
        return ;
    increase = max(min, this->reserve);
    increase = (increase + 0xfff) & ~0xfff;
    for (u32 i = this->start + this->reserve; i < this->start + this->reserve + increase; i += PAGESIZE)
        mem.map(i, 1, 1);
    this->reserve += increase;
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG "%5gkheap%r: increased kheap reserve from 0x%x to 0x%x\n", this->reserve - increase, this->reserve);
#endif
}

void *kheap::alloc(u32 size) { return this->alloc(size, 0); }

void *kheap::alloc(u32 size, u32 flags)
{
    void *out;

    if (flags & ALLOC_ALIGNED)
    {
        this->free_zone = (this->free_zone + 0xfff) & ~0xfff;
    }
    out = (void*)this->free_zone;
    if (this->reserve < this->free_zone - this->start + size)
        this->expand(size);
    this->free_zone += size;
    if (flags & ALLOC_ZEROED)
        memset(out, 0, size);
#ifdef DEBUG_KHEAP
    term.printk(KERN_DEBUG "%5gkheap%r: alloc %p(s:%d, f:%c%c) (rem: %p/%p)\n", out, size, flags & ALLOC_ALIGNED ? 'A' : ' ', flags & ALLOC_ZEROED ? '0' : ' ', this->free_zone - this->start, this->reserve);
#endif
    return out;
}
