#pragma once

#include <common.h>

usize grow_capacity(usize capacity);

#define ALLOCATE(type, count) (type*)reallocate(NULL, sizeof(type) * (count))

void* reallocate(void* source, usize new_size);
