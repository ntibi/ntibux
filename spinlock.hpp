#ifndef SPINLOCK_HPP
# define SPINLOCK_HPP

#include "header.hpp"
#include "misc.hpp"


class spinlock
{
public:
    spinlock();
    void lock();
    void release();
    u32 locked;
};

#endif
