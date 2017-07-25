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

