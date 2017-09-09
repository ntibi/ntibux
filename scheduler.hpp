#ifndef SCHEDULER_HPP
# define SCHEDULER_HPP

#include "kernel.hpp"

class task
{
    u32 id;
    u32 eip;
    u32 esp, ebp;
    page_directory *pd;

    list tasks;
};

class scheduler
{
    void init();
};

#endif
