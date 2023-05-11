#include <stdio.h>

#include "value.h"
#include <utils/memory.h>

ARRAY_INIT(value)

void append_value_array(value_array_t* const instance, const value_t value)
{
    if (instance->capacity < instance->count + 1UL)
    {
        const size_t old_capacity = instance->capacity;

        instance->capacity = grow_capacity(old_capacity);

        value_t* const values = GROW_ARRAY(
            value_t, instance->values, old_capacity, instance->capacity);

        instance->values = values;
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
