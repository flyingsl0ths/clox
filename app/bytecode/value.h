#pragma once

#include <common.h>
#include <utils/array.h>

typedef enum
{
    VAL_NIL,
    VAL_BOOL,
    VAL_NUM,
    VAL_OBJ
} value_type_t;

typedef struct object_t     object_t;
typedef struct obj_string_t obj_string_t;
typedef struct table_t      table_t;

typedef struct
{
    value_type_t type;
    union
    {
        bool      boolean;
        f64       number;
        object_t* obj;
    } as;
} value_t;

static const value_t NIL_VAL = {.type = VAL_NIL, .as = {.obj = NULL}};

typedef value_t (*binary_operator_t)(const value_t, const value_t);

ARRAY(value_t, value)

static inline bool is_bool(const value_t value) { return value.type == VAL_BOOL; }

static inline bool is_nil(const value_t value) { return value.type == VAL_NIL; }

static inline bool is_number(const value_t value) { return value.type == VAL_NUM; }

static inline bool is_zero(const value_t value)
{
    return value.type == VAL_NUM && value.as.number == 0.0;
}

static inline bool      is_obj(const value_t value) { return value.type == VAL_OBJ; }

static inline bool      as_bool(const value_t value) { return value.as.boolean; }

static inline f64       as_number(const value_t value) { return value.as.number; }

static inline object_t* as_obj(const value_t value) { return value.as.obj; }

static inline value_t   from_bool(const bool value)
{
    return (value_t){.type = VAL_BOOL, .as = {.boolean = value}};
}

static inline value_t from_object(object_t* const value)
{
    return (value_t){.type = VAL_OBJ, .as = {.obj = value}};
}

static inline value_t nil(void)
{
    return (value_t){.type = VAL_NIL, .as = {.number = 0}};
}

static inline value_t from_number(const f64 value)
{
    return (value_t){.type = VAL_NUM, .as = {.number = value}};
}

void    init_value_array(value_array_t* self, usize size);

void    append_value_array(value_array_t* self, value_t value);

void    free_value_array(value_array_t* self);

void    print_value(const value_t* self);

value_t values_add(value_t left, value_t right, object_t* objects, table_t* strings);

value_t values_sub(value_t left, value_t right);

value_t values_multiply(value_t left, value_t right);

value_t values_divide(value_t left, value_t right);

value_t values_equal(value_t left, value_t right);

value_t values_greater(value_t left, value_t right);

value_t values_less(value_t left, value_t right);
