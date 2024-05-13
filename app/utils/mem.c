#include <stdlib.h>

#include "mem.h"

size_t grow_capacity(const size_t capacity)
{
    return capacity < 8UL ? 8UL : capacity * 2UL;
}

void* reallocate(void* const source, const size_t new_size)
{
    if (new_size == 0UL && source)
    {
        free(source);
        return NULL;
    }

    void* const result = realloc(source, new_size);

    if (!result) { exit(EXIT_FAILURE); }

    return result;
}
