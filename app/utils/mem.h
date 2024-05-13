#pragma once

#include <common.h>

size_t grow_capacity(size_t capacity);

#define ALLOCATE(type, count) (type*)reallocate(NULL, sizeof(type) * (count))

void* reallocate(void* source, size_t new_size);
