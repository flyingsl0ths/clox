#pragma once

#include <common.h>

size_t grow_capacity(size_t capacity);

#define GROW_ARRAY(type, source, old_count, new_count)                         \
    (type*)reallocate(                                                         \
        source, sizeof(type) * (old_count), sizeof(type) * (new_count))

#define FREE_ARRAY(type, source, old_count)                                    \
    reallocate(source, sizeof(type) * (old_count), 0)

void* reallocate(void* source, size_t old_size, size_t new_size);
