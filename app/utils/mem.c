#include <stdlib.h>

#include "mem.h"

usize grow_capacity(const usize capacity)
{
    return capacity < 8UL ? 8UL : capacity * 2UL;
}

void* reallocate(void* const source, const usize new_size)
{
    if (new_size == 0UL && source)
    {
        free(source);
        return NULL;
    }

    void* const result = realloc(source, new_size);

    if (!result) { return NULL; }

    return result;
}
