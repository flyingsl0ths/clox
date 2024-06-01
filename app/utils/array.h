#include <stddef.h>
#include <stdlib.h>

#include <common.h>

#define ARRAY(type, type_name)                                                 \
    typedef struct                                                             \
    {                                                                          \
        size_t count;                                                          \
        size_t capacity;                                                       \
        type*  values;                                                         \
    } type_name##_array_t;

#define ARRAY_INIT(type_name, elem_type)                                       \
    void init_##type_name##_array(type_name##_array_t* const instance,         \
                                  const size_t               size)             \
    {                                                                          \
        instance->count    = 0;                                                \
        instance->capacity = size;                                             \
        instance->values =                                                     \
            size > 0 ? ((elem_type*)malloc(sizeof(elem_type) * size)) : NULL;  \
    }

#define GROW_ARRAY(type, source, new_count)                                    \
    (type*)reallocate(source, sizeof(type) * (new_count))

#define FREE_ARRAY(source)                                                     \
    reallocate(source->values, 0UL);                                           \
    source->values   = NULL;                                                   \
    source->count    = 0;                                                      \
    source->capacity = 0;
