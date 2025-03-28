#pragma once

#include "value.h"
#include <common.h>

typedef enum
{
    OBJ_STRING
} object_type_t;

struct object_t
{
    object_type_t    type;
    struct object_t* next;
};

struct obj_string_t
{
    object_t    obj;
    usize       length;
    const char* chars;
    uint32_t    hash;
};

obj_string_t* copy_string(str chars, usize length, object_t* objects, table_t* strings);

obj_string_t* take_string(char* chars, usize length, object_t* objects, table_t* strings);

static inline object_type_t object_type(const value_t value)
{
    return as_obj(value)->type;
}

static inline bool is_object_type(const value_t value, const object_type_t type)
{
    const bool result = is_obj(value) && as_obj(value)->type == type;
    return result;
}

static inline bool is_string(const value_t value)
{
    return is_object_type(value, OBJ_STRING);
}

static inline obj_string_t* as_string(const value_t value)
{
    return (obj_string_t*)as_obj(value);
}

static inline const char* as_c_str(const value_t value)
{
    return ((obj_string_t*)as_obj(value))->chars;
}

void print_object(value_t value);

s32  string_cmp(value_t left, value_t right);

obj_string_t*
     strings_add(value_t left, value_t right, object_t* objects, table_t* strings);

void free_object(object_t* object);
