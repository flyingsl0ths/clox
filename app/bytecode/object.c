#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode/value.h"
#include "object.h"
#include <utils/mem.h>

object_t* allocate_object(usize size, const object_type_t type)
{
    object_t* const obj = (object_t*)reallocate(NULL, size);
    obj->type           = type;
    return obj;
}

#define ALLOCATE_OBJ(type, object_type)                                        \
    (type*)allocate_object(sizeof(type), object_type)

obj_string_t* allocate_string(const char* const chars, usize length)
{
    obj_string_t* const string = ALLOCATE_OBJ(obj_string_t, OBJ_STRING);

    string->length             = length;
    string->chars              = chars;

    return string;
}

obj_string_t* copy_string(const char* const chars, usize length)
{
    char* heap_chars = ALLOCATE(char, length - 1UL);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';
    return allocate_string(heap_chars, length);
}

void print_object(const value_t value)
{
    switch (object_type(value))
    {
        case OBJ_STRING: printf("\"%s\"\n", as_c_str(value)); break;
    }
}

bool strings_equal(value_t left, value_t right)
{
    const obj_string_t* const first  = as_string(left);
    const obj_string_t* const second = as_string(right);

    return (first->length == second->length) &&
           (memcmp(first->chars, second->chars, first->length) == 0);
}

obj_string_t* take_string(char* chars, usize length)
{
    return allocate_string(chars, length);
}

obj_string_t* strings_add(const value_t left, const value_t right)
{
    const obj_string_t* const first  = as_string(left);
    const obj_string_t* const second = as_string(right);

    const usize               length = first->length + second->length;

    char*                     chars  = ALLOCATE(char, length + 1);

    memcpy(chars, first->chars, first->length);
    memcpy(chars + first->length, second->chars, second->length);

    chars[length]              = '\0';

    obj_string_t* const result = take_string(chars, length);

    chars                      = NULL;

    return result;
}

void free_string_object(object_t* const obj)
{
    obj_string_t* st = (obj_string_t*)obj;
    free((void*)st->chars);
    st->chars  = NULL;
    st->length = 0;
}

void free_object(const value_t value)
{
    if (!is_obj(value)) return;

    object_t* obj = as_obj(value);

    if (obj->type == OBJ_STRING) { free_string_object(obj); }

    free((void*)obj);
}
