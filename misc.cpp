#include "misc.hpp"

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        ++len;
    return len;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char*)dest;
    const char *s = (char*)src;

    while (n)
    {
        *d = *s;
        ++d;
        ++s;
        --n;
    }
    return dest;
}

void *memset(void *s, int c, size_t n)
{
    char *p = (char*)s;
    while (n)
    {
        *p++ = c;
        --n;
    }
    return s;
}

int is_sep(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

int is_print(char c)
{
    return (c >= 0x20 && c <= 0x7e);
}

char *strcpy(char *dest, const char *src)
{
    size_t i;

    for (i = 0; src[i]; ++i)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i]; ++i)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }
    return *s2 - *s1;
}

inline u32 is_upper(char c) { return (c >= 'A' && c <= 'Z'); }
inline u32 is_lower(char c) { return (c >= 'a' && c <= 'z'); }
inline u32 is_digit(char c) { return (c >= '0' && c <= '9'); }
inline u32 is_bdigit(char c) { return (c >= '0' && c <= '1'); }
inline u32 is_odigit(char c) { return (c >= '0' && c <= '7'); }
inline u32 is_xdigit(char c) { return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')); }
inline char to_upper(char c) { return is_lower(c) ? c + ('A' - 'a') : c; }
inline char to_lower(char c) { return is_upper(c) ? c + ('a' - 'A'): c; }

u32 atoi(const char *s, u32 base)
{
    static u32 (*is_digit_base[]) (char) = {NULL, NULL, is_bdigit, NULL, NULL, NULL, NULL, NULL, is_odigit, NULL, is_digit, NULL, NULL, NULL, NULL, NULL, is_xdigit};
    u32 out = 0;

    switch (base)
    {
        case 16: // 0x...
        case 2:  // 0b...
            s += 1;
        case 8:  // 0...
            s += 1;
        default: break;
    }
    while (is_digit_base[base](*s))
    {
        out = out * base + (to_lower(*s) - (is_lower(*s) ? 'a' - 10 : '0'));
        ++s;
    }
    return out;
}

u32 atoi(const char *s)
{
    u32 base = 10;

    if (s[0] == '0')
    {
        if (to_lower(s[1]) == 'x')
            base = 16;
        else if (to_lower(s[1]) == 'b')
            base = 2;
        else
            base = 8;
    }
    return atoi(s, base);
}
