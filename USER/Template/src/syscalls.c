/**
 * Minimal stubs for freestanding bare-metal STM32 (no newlib required).
 * Provides _sbrk for heap allocation (malloc) and basic errno.
 */

#include <stddef.h>

#undef errno
int errno;

extern char _end;
static char *heap_ptr = (char *)&_end;

/* requested but not used */
#define ENOMEM 12
#define EBADF  9

void *_sbrk(ptrdiff_t incr)
{
    char *prev = heap_ptr;
    char *stack;
    __asm volatile ("mov %0, sp" : "=r" (stack));
    if (heap_ptr + incr > stack)
        return (void *)-1;
    heap_ptr += incr;
    return (void *)prev;
}
