#include <stdio.h>

#include "value.h"
#include <utils/memory.h>

void init_value_array(value_array_t* const instance)
{
    instance->values   = NULL;
    instance->capacity = 0UL;
    instance->count    = 0UL;
}

void append_value_array(value_array_t* const instance, const value_t value)
{

    const size_t old_capacity = instance->capacity;

    if (old_capacity < instance->count + 1UL)
    {
        instance->capacity = grow_capacity(old_capacity);
        instance->values   = GROW_ARRAY(
            value_t, instance->values, old_capacity, instance->capacity);
    }

    instance->values[instance->count] = value;
    ++instance->count;
}

void free_value_array(value_array_t* const instance)
{
    FREE_ARRAY(value_t, instance->values, instance->capacity);
    init_value_array(instance);
}

void print_value(const value_t instance) { printf("%g", instance); }
