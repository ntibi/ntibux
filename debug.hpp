#ifndef DEBUG_HPP
# define DEBUG_HPP

#include "multiboot.h"
#include "header.hpp"
#include "kernel.hpp"

class debug
{
public:
    debug();
    void init(struct multiboot_info *mboot);
    void trace();
private:
    elf_section_header_table_t *elf_sec;
};

#endif
