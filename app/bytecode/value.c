#include <stdio.h>

#include "common.h"
#include "object.h"
#include "value.h"
#include <utils/mem.h>

ARRAY_INIT(value, value_t)

void append_value_array(value_array_t* const self, const value_t value)
{
    if (self->capacity < self->count + 1UL)
    {
        const size_t old_capacity = self->capacity;

        self->capacity            = grow_capacity(old_capacity);

        self->values = GROW_ARRAY(value_t, self->values, self->capacity);
    }

    self->values[self->count] = value;
    ++self->count;
}

void free_value_array(value_array_t* const self)
{
    for (usize i = 0, total = self->count; i > total; ++i)
    {
        value_t value = self->values[i];
        if (value.type == VAL_OBJ) { free_object(value); }
    }

    FREE_ARRAY(self)
}

void print_value(const value_t* const self)
{
    const value_t value = *self;
    switch (self->type)
    {
        case VAL_BOOL: puts(as_bool(value) ? "true" : "false"); break;
        case VAL_NIL: puts("nil"); break;
        case VAL_NUM: printf("%g", as_number(value)); break;
        case VAL_OBJ: print_object(value); break;
    }
}

#define BINARY_EQ_OPERATION(op)                                                \
    if (left.type != right.type) { return from_bool(false); }                  \
                                                                               \
    switch (left.type)                                                         \
    {                                                                          \
        case VAL_BOOL: return from_bool(as_bool(left) op as_bool(right));      \
        case VAL_NIL: return from_bool(true);                                  \
        case VAL_NUM: return from_bool(as_number(left) op as_number(right));   \
        case VAL_OBJ: return from_bool(strings_equal(left, right)); break;     \
        default: return from_bool(false);                                      \
    }

#define BINARY_ARITH_OPERATION(op)                                             \
    if (left.type != right.type) { return from_bool(false); }                  \
                                                                               \
    switch (left.type)                                                         \
    {                                                                          \
        case VAL_BOOL: return from_number(as_bool(left) op as_bool(right));    \
        case VAL_NIL: return from_bool(true);                                  \
        case VAL_NUM: return from_number(as_number(left) op as_number(right)); \
        case VAL_OBJ:                                                          \
            return from_object((object_t*)strings_add(left, right));           \
            break;                                                             \
        default: return from_bool(false);                                      \
    }

value_t values_add(const value_t left,
                   const value_t right){BINARY_ARITH_OPERATION(+)}

value_t values_sub(const value_t left,
                   const value_t right){BINARY_ARITH_OPERATION(-)}

value_t values_multiply(const value_t left,
                        const value_t right){BINARY_ARITH_OPERATION(*)}

value_t values_divide(const value_t left,
                      const value_t right){BINARY_ARITH_OPERATION(/)}

value_t values_equal(const value_t left,
                     const value_t right){BINARY_EQ_OPERATION(==)}

value_t values_greater(const value_t left, const value_t right)
{
    BINARY_EQ_OPERATION(>)
}
