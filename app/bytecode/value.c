#include <stdio.h>

#include "value.h"
#include <utils/memory.h>

ARRAY_INIT(value)

void append_value_array(value_array_t* const self, const value_t value)
{
    if (self->capacity < self->count + 1UL)
    {
        const size_t old_capacity = self->capacity;

        self->capacity = grow_capacity(old_capacity);

        value_t* const values =
            GROW_ARRAY(value_t, self->values, old_capacity, self->capacity);

        self->values = values;
    }

    self->values[self->count] = value;
    ++self->count;
}

void free_value_array(value_array_t* const self)
{
    FREE_ARRAY(value_t, self->values, self->capacity);
    init_value_array(self);
}

void print_value(const value_t self) { printf("%g", self); }

value_t add_values(value_t left, value_t right) { return left + right; }

value_t sub_values(value_t left, value_t right) { return left - right; }

value_t mul_values(value_t left, value_t right) { return left * right; }

value_t div_values(value_t left, value_t right) { return left / right; }
