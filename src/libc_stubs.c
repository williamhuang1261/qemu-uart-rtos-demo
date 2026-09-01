/*
 * This firmware links with -nostdlib (no newlib, no syscall stubs to fake
 * a hosted environment) so only the handful of libc functions FreeRTOS
 * actually calls are implemented here, by hand, freestanding.
 */
#include <stddef.h>

void *memset(void *dst, int value, size_t count)
{
    unsigned char *d = (unsigned char *)dst;
    while (count--) {
        *d++ = (unsigned char)value;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t count)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (count--) {
        *d++ = *s++;
    }
    return dst;
}

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}
