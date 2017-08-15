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

mem::mem() : kheap(), total(0) { }

void mem::init(u32 high_mem)
{
    this->total = high_mem * 1024;
    term.printk(KERN_INFO "detected mem: %uMB\n", this->total >> 20);

    this->kheap.init();
    this->frames.init(this->total / PAGESIZE);

    this->kernel_pd = (page_directory*)this->kheap.alloc(sizeof(page_directory), ALLOC_ALIGNED | ALLOC_ZEROED);
    this->identity_map_kernel();
    term.printk(KERN_DEBUG "activating paging...\n");
    this->switch_page_directory(this->kernel_pd);
}

void mem::identity_map_kernel()
{
    for (u32 i = 0; i < kend; i += 0x1000) // kernel identity mapping
    {
        this->map(i, i, this->kernel_pd, 1, 1);
    }
    term.printk(KERN_INFO "identity mapped the %d first frames (0x%x - 0x%x)\n", ((kend + 0xfff) & 0xfffff000) / 0x1000, 0, ((kend + 0xfff) & 0xfffff000) - 1);
}

u32 mem::map(u32 vaddr, page_directory *pd, u32 kernel, u32 writeable)
{
    vaddr &= 0xfffff000;
    return this->alloc_frame(this->get_page(vaddr, pd), kernel, writeable);
}

u32 mem::map(u32 vaddr, u32 paddr, page_directory *pd, u32 kernel, u32 writeable)
{
    vaddr &= 0xfffff000;
    paddr &= 0xfffff000;
    return this->alloc_frame(this->get_page(vaddr, pd), paddr, kernel, writeable);
}

page *mem::get_page(u32 address, page_directory *pd)
{
    u32 directory_index;
    u32 table_index;
    // adddress    : xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // dir index   : 1111111111                      
    // table index :           1111111111            
    // offset      :                     111111111111

    directory_index = (address >> 22) & 0x3ff;
    table_index = (address >> 12) & 0x3ff;
    if (!pd->tables[directory_index])
    {
        page_table *pt = (page_table*)this->kheap.alloc(sizeof(page_table), ALLOC_ALIGNED | ALLOC_ZEROED);
        pd->tables[directory_index] = pt;
        pd->paddrs[directory_index] = (u32)pt | 0x7; // TODO: get real paddr (when paging will be activated)
    }
    return &pd->tables[directory_index]->pages[table_index];
}

void mem::switch_page_directory(struct page_directory *pd)
{
    asm volatile (
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            :: "r"(pd->paddrs));
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
        term.printk(KERN_ERROR "frame 0x%x already mapped to a page\n", frame);
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


kheap::kheap() : free_zone(0) { }

void kheap::init()
{
    this->free_zone = kend;
}

void *kheap::alloc(u32 size) { return this->alloc(size, 0); }

void *kheap::alloc(u32 size, u32 flags)
{
    void *out;

    if (flags & ALLOC_ALIGNED)
    {
        this->free_zone &= 0xfffff000;
        this->free_zone += 0x1000;
    }
    out = (void*)this->free_zone;
    this->free_zone += size;
    if (flags & ALLOC_ZEROED)
        memset(out, 0, size);
    return out;
}
