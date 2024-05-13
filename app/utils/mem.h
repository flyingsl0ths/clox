#pragma once

#include <common.h>

size_t grow_capacity(size_t capacity);

#define ALLOCATE(type, count) (type*)reallocate(NULL, sizeof(type) * (count))

#define GROW_ARRAY(type, source, new_count)                                    \
    (type*)reallocate(source, sizeof(type) * (new_count))

#define FREE_ARRAY(type, source) reallocate(source, 0UL)

void* reallocate(void* source, size_t new_size);
