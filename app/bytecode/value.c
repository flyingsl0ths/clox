#include <stdio.h>

#include "object.h"
#include "value.h"
#include <utils/mem.h>

ARRAY_INIT(value, value_t)

void append_value_array(value_array_t* const self, const value_t value)
{
    if (self->capacity < self->count + 1UL)
    {
        const usize old_capacity = self->capacity;

        self->capacity           = grow_capacity(old_capacity);

        self->values = GROW_ARRAY(value_t, self->values, self->capacity);
    }

    self->values[self->count] = value;
    ++self->count;
}

void free_value_array(value_array_t* const self) { FREE_ARRAY(self) }

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
    switch (left.type)                                                         \
    {                                                                          \
        case VAL_BOOL: return from_bool(as_bool(left) op as_bool(right));      \
        case VAL_NIL: return from_bool(true);                                  \
        case VAL_NUM: return from_bool(as_number(left) op as_number(right));   \
        case VAL_OBJ: return from_bool(strings_equal(left, right)); break;     \
        default: return from_bool(false);                                      \
    }

#define BINARY_ARITH_OPERATION(op)

value_t values_add(value_t         left,
                   value_t         right,
                   object_t* const objects,
                   table_t* const  strings)
{
    switch (left.type)
    {
        case VAL_BOOL: return from_number(as_bool(left) + as_bool(right));
        case VAL_NIL: return from_number(0);
        case VAL_NUM: return from_number(as_number(left) + as_number(right));
        case VAL_OBJ:
            return from_object(
                (object_t*)strings_add(left, right, objects, strings));
        default: return from_bool(false);
    }
}

value_t values_sub(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_NUM: return from_number(as_number(left) - as_number(right));
        default: return from_bool(false);
    }
}

value_t values_multiply(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_NUM: return from_number(as_number(left) * as_number(right));
        default: return from_bool(false);
    }
}

value_t values_divide(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_NUM: return from_number(as_number(left) / as_number(right));
        default: return from_bool(false);
    }
}

value_t values_equal(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_BOOL: return from_bool(as_bool(left) == as_bool(right));
        case VAL_NIL: return from_bool(true);
        case VAL_NUM: return from_bool(as_number(left) == as_number(right));
        case VAL_OBJ: return from_bool(as_obj(left) == as_obj(right)); break;
        default: return from_bool(false);
    }
}

value_t values_greater(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_NUM: return from_bool(as_number(left) > as_number(right));
        case VAL_OBJ: return from_bool(string_cmp(left, right) > 0); break;
        default: return from_bool(false);
    }
}

value_t values_less(const value_t left, const value_t right)
{
    switch (left.type)
    {
        case VAL_NUM: return from_bool(as_number(left) < as_number(right));
        case VAL_OBJ: return from_bool(string_cmp(left, right) < 0); break;
        default: return from_bool(false);
    }
}
