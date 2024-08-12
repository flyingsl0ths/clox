#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include <utils/mem.h>
#include <utils/table.h>

void update_objects_ptr(object_t* objects, object_t* const obj)
{
    if (objects == NULL) { objects = obj; }
    else
    {
        obj->next = objects;
        objects   = obj;
    }
}

object_t*
allocate_object(const usize size, const object_type_t type, object_t* objects)
{
    object_t* const obj = (object_t*)reallocate(NULL, size);
    obj->type           = type;
    obj->next           = NULL;

    update_objects_ptr(objects, obj);

    return obj;
}

#define ALLOCATE_OBJ(type, object_type, objects)                               \
    (type*)allocate_object(sizeof(type), object_type, objects)

obj_string_t* allocate_string(str const       chars,
                              const usize     length,
                              const uint32_t  hash,
                              object_t* const objects,
                              table_t* const  strings)
{
    obj_string_t* const string =
        ALLOCATE_OBJ(obj_string_t, OBJ_STRING, objects);

    string->length = length;
    string->chars  = chars;
    string->hash   = hash;

    table_set(strings, string, NIL_VAL);

    return string;
}

uint32_t hash_string(str const chars, const usize size)
{
    uint32_t hash = 2166136261u;

    for (usize i = 0; i < size; ++i)
    {
        hash ^= (uint8_t)chars[i];
        hash *= 16777619;
    }

    return hash;
}

obj_string_t* copy_string(str const       chars,
                          const usize     length,
                          object_t* const objects,
                          table_t* const  strings)
{
    const uint32_t      hash = hash_string(chars, length);

    obj_string_t* const interned =
        table_find_string(strings, chars, length, hash);

    if (interned != NULL) return interned;

    char* heap_chars = ALLOCATE(char, length - 1UL);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';

    return allocate_string(heap_chars, length, hash, objects, strings);
}

void print_object(const value_t value)
{
    switch (object_type(value))
    {
        case OBJ_STRING: printf("\"%s\"\n", as_c_str(value)); break;
    }
}

s32 string_cmp(const value_t left, const value_t right)
{
    const obj_string_t* const first  = as_string(left);
    const obj_string_t* const second = as_string(right);

    return (first->length == second->length) &&
           (memcmp(first->chars, second->chars, first->length));
}

obj_string_t* take_string(char*           chars,
                          const usize     length,
                          object_t* const objects,
                          table_t* const  strings)
{
    const uint32_t      hash = hash_string(chars, length);

    obj_string_t* const interned =
        table_find_string(strings, chars, length, hash);

    if (interned != NULL)
    {
        free(chars);
        return interned;
    }

    return allocate_string(chars, length, hash, objects, strings);
}

obj_string_t* strings_add(const value_t   left,
                          const value_t   right,
                          object_t* const objects,
                          table_t* const  strings)
{
    const obj_string_t* const first  = as_string(left);
    const obj_string_t* const second = as_string(right);

    const usize               length = first->length + second->length;

    char*                     chars  = ALLOCATE(char, length + 1);

    memcpy(chars, first->chars, first->length);
    memcpy(chars + first->length, second->chars, second->length);

    chars[length]              = '\0';

    obj_string_t* const result = take_string(chars, length, objects, strings);

    chars                      = NULL;

    return result;
}

void free_string_object(object_t* const obj)
{
    obj_string_t* const st = (obj_string_t*)obj;
    free((void*)st->chars);
    st->chars  = NULL;
    st->length = 0;
}

void free_object(object_t* const object)
{
    if (object->type == OBJ_STRING) { free_string_object(object); }

    free((void*)object);
}
