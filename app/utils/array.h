#include <common.h>

#define ARRAY(type, type_name)                                                 \
    typedef struct                                                             \
    {                                                                          \
        size_t count;                                                          \
        size_t capacity;                                                       \
        type*  values;                                                         \
    } type_name##_array_t

#define ARRAY_INIT(type_name)                                                  \
    void init_##type_name##_array(type_name##_array_t* const instance)         \
    {                                                                          \
        instance->count    = 0;                                                \
        instance->capacity = 0;                                                \
        instance->values   = NULL;                                             \
    }\
