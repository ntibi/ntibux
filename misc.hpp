#ifndef MISC_HPP
# define MISC_HPP

#include <stddef.h>
#include <stdint.h>
#include "header.hpp"


// misc:
size_t strlen(const char *str);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int is_sep(char c);
int is_print(char c);


// kmisc:
void trace();

inline void outb(u16 port, u8 val)
{
    asm volatile ( "outb %1, %0" : : "a"(val), "Nd"(port)  );
}

inline u8 inb(u16 port)
{
    u8 ret;
    asm volatile ( "inb %0, %1" : "=a"(ret) : "Nd"(port) );
    return ret;
}

template<typename T>
inline const T& min(const T& a, const T& b) { if (a < b) return a; return b; }

template<typename T>
inline const T& max(const T& a, const T& b) { if (a > b) return a; return b; }

#endif
