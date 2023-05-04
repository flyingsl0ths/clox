#include "chunk.h"
#include <utils/memory.h>

void init_chunk(chunk_t* const instance)
{
    instance->count    = 0;
    instance->capacity = 0;
    instance->code     = NULL;
    instance->lines    = NULL;
    init_value_array(&instance->constants);
}

void free_chunk(chunk_t* instance)
{

    FREE_ARRAY(byte, instance->code, instance->capacity);
    FREE_ARRAY(u32, instance->lines, instance->capacity);
    free_value_array(&instance->constants);
    init_chunk(instance);
}

void write_chunk(chunk_t* instance, const byte data, const u32 line)
{
    const size_t old_capacity = instance->capacity;
    if (old_capacity < instance->count + 1)
    {
        instance->capacity = grow_capacity(old_capacity);

        instance->code =
            GROW_ARRAY(byte, instance->code, old_capacity, instance->capacity);

        instance->lines =
            GROW_ARRAY(u32, instance->lines, old_capacity, instance->capacity);
    }

    instance->code[instance->count]  = data;
    instance->lines[instance->count] = line;
    ++instance->count;
}

size_t add_constant(chunk_t* const instance, const value_t value)
{
    append_value_array(&instance->constants, value);
    return instance->constants.count - 1;
}
