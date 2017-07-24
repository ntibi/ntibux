#include "mem.hpp"

extern u32 end;

static inline u32 off(u32 b) { return b % 32; }
static inline u32 index(u32 b) { return b / 32; }

void page::claim(u32 frame, u32 kernel, u32 writeable)
{
    this->present = 1;
    this->frame = frame >> 12;
    this->w = !!writeable;
    this->user = !kernel;
}

void page::free(void)
{
    if (!this->frame)
    {
        term.printk(KERN_ERROR "freeing non allocated frame (0x%x)\n", this->frame);
        return ;
    }
    this->frame = 0;
}


class mem mem;

mem::mem() : kheap(), total(0) { }

void mem::init(u32 high_mem)
{
    u32 i;

    this->total = high_mem * 1024;
    term.printk(KERN_INFO "detected mem: %uMB\n", this->total >> 20);

    this->kheap.init();
    this->frames.init(this->total / PAGESIZE);

    for (i = 0; i < (u32)&end + 0x1000 * 100; i += 0x1000)
    {
        this->alloc_frame(this->get_page(i, &this->kernel_pd), 0, 0);
    }
    term.printk(KERN_INFO "identity mapped the first %d frames\n", (u32)&end / 0x1000);
    term.printk(KERN_INFO "switching to kernel page directory...");
    term.getchar();
    this->switch_page_directory(&this->kernel_pd);
    term.getchar();
    term.printk(" done\n");
    // u32 *ptr = (u32*)0xA0000000; // TODO: ca devrait planter
    // u32 pf = *ptr;
    // term.printk("%d\n", pf);
}

page *mem::get_page(u32 address, page_directory *pd)
{
    u32 pdn;

    address >>= 12;
    term.printk(KERN_DEBUG "get_page(0x%x)\n", address);
    pdn = address >> 10;
    if (pd->tables[pdn])
    {
        return &pd->tables[pdn]->pages[address & 0xfff];
    }
    else
    {
        pd->tables[pdn] = (page_table*)this->kheap.alloc(sizeof(page_table), ALLOC_ALIGNED | ALLOC_ZEROED);
        pd->paddr[pdn] = (u32)pd->tables[pdn] | 0x7;
        return &pd->tables[pdn]->pages[address & 0xfff];
    }
    return 0;
}

void mem::switch_page_directory(struct page_directory *pd)
{
    // asm volatile("mov %0, %%cr3":: "r"(&pd->paddr));
    // u32 cr0;
    // asm volatile("mov %%cr0, %0": "=r"(cr0));
    // cr0 |= 0x80000000; // Enable paging!
    // asm volatile("mov %0, %%cr0":: "r"(cr0));

    asm volatile (
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            :: "r"(&pd->paddr));
}

void frames::init(u32 nframes)
{
    this->nframes = nframes;
    this->frames = (u32*)mem.kheap.alloc(index(this->nframes), ALLOC_ZEROED);
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
            if (!frames[0x1 << j])
                return (i*32 + j) * 0x1000;
        }
    }
    return 0;
}

void mem::alloc_frame(page *p, u32 kernel, u32 writeable)
{
    u32 frame;

    frame = this->frames.get_free_frame();
    this->frames.mark_frame(frame);
    p->claim(frame, kernel, writeable);
}

void mem::free_frame(page *p)
{
    this->frames.unmark_frame(p->frame << 12);
    p->free();
}


kheap::kheap() : free_zone(0) { }

void kheap::init()
{
    this->free_zone = (u32)&end;
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
