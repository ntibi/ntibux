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
char *strchr(const char *s, int c);
inline u32 is_sep(char c) { return (c == ' ' || c == '\t' || c == '\n'); }
inline u32 is_print(char c) { return (c >= 0x20 && c <= 0x7e); }
inline u32 is_upper(char c) { return (c >= 'A' && c <= 'Z'); }
inline u32 is_lower(char c) { return (c >= 'a' && c <= 'z'); }
inline u32 is_digit(char c) { return (c >= '0' && c <= '9'); }
inline u32 is_bdigit(char c) { return (c >= '0' && c <= '1'); }
inline u32 is_odigit(char c) { return (c >= '0' && c <= '7'); }
inline u32 is_xdigit(char c) { return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')); }
inline char to_upper(char c) { return is_lower(c) ? c + ('A' - 'a') : c; }
inline char to_lower(char c) { return is_upper(c) ? c + ('a' - 'A'): c; }

u32 atoi(const char *s); // call with a base prefix (e.g. 0xdab)
u32 atoi(const char *s, u32 base); // call without a base prefix (e.g. "cafebabe", 16)

// kmisc:
void panic(const char fn[], u32 line, const char msg[]);
#define PANIC(msg) panic(__FILE__, __LINE__, msg)
#define UNREACHABLE PANIC("unreachable code reached")

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
