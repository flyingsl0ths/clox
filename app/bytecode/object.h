#pragma once

#include <bytecode/value.h>
#include <common.h>

typedef enum
{
    OBJ_STRING
} object_type_t;

struct object_t
{
    object_type_t type;
};

struct obj_string_t
{
    object_t    obj;
    usize       length;
    const char* chars;
    uint32_t    hash;
};

obj_string_t*               copy_string(str chars, const usize length);

obj_string_t*               take_string(str chars, const usize length);

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

void          print_object(const value_t value);

bool          strings_equal(const value_t left, const value_t right);

obj_string_t* strings_add(const value_t left, const value_t right);

void          free_object(const value_t value);
