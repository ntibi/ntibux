#include "kernel.hpp"
#include "terminal.hpp"

#ifndef GDT_HPP
# define GDT_HPP

class terminal;


class GDT
{
public:
   GDT();
   void init(void);

   struct header
   {
       u16 size;
       void *offset;
   } __attribute__((packed));
   struct entry;
   struct flags;
   struct access;


private:
   void load_gdt();
   void set_gate(u8 n, u32 base, u32 limit, u8 access, u8 flags);

   struct header gdt_ptr;
   struct entry *gdt_entries;
   u8 gdt_entries_size;
};


struct GDT::entry
{
    u16 low_limit;
    u16 low_base;
    u8 mid_base;
    u8 access; // struct GDT::access
    u8 high_limit : 4;
    u8 flags : 4; // struct GDT::flags
    u8 high_base;
} __attribute__((packed));

struct GDT::flags
{
    flags(u8 raw) : raw(raw & 0xf) { };
    union
    {
        struct
        {
            u8 z1 : 1; // 0
            u8 z2 : 1; // 0
            u8 sz : 1; // (0: 16B, 1: 32B)
            u8 gr : 1; // (0: limit is 1B blocks, 1: limit is 4KB blocks)
        };
        u8 raw : 4;
    };
    static const u8 MODE_16B = 0 << 2;
    static const u8 MODE_32B = 1 << 2;
    static const u8 GRANULARITY_1KB = 0 << 3;
    static const u8 GRANULARITY_4KB = 1 << 3;
} __attribute__((packed));

struct GDT::access
{
    access(u8 raw) : raw(raw | DEF) { };
    union
    {
        struct
        {
            u8 ac : 1; // 0 (cpu will set it to 1 when accessed)
            // if code: readable ?
            u8 rw : 1; // if data: writeable ?
            // if code: exec rights (1: ring <= current, 0: only by ring set in privl)
            u8 dc : 1; // if data: growing dir (0: up, 1: down)
            u8 ex : 1; // is executable (0: data, 1: code)
            u8 one : 1; // 1
            u8 privl : 2; // ring level (0-3)
            u8 pr : 1; // 1 (present)
        };
        u8 raw;
    };
    static const u8 DEF = (1 << 7) | (1 << 4) | (0 << 0);
    static const u8 WRITEABLE_DATA = 1 << 1;
    static const u8 READABLE_CODE = 1 << 1;
    static const u8 GROWS_UP = 0 << 2;
    static const u8 GROWS_DOWN = 1 << 2;
    static const u8 CONFORMING= 1 << 2;
    static const u8 NON_CONFORMING = 0 << 2;
    static const u8 CODE = 1 << 3;
    static const u8 DATA = 0 << 3;
    static const u8 KERNEL = 0 << 5;
    static const u8 USER = 3 << 5;
} __attribute__((packed));

#endif
