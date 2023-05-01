
#include "chunk.h"
#include <utils/memory.h>

void init_chunk(chunk_t* const instance)
{
    instance->count    = 0;
    instance->capacity = 0;
    instance->code     = NULL;
}

void free_chunk(chunk_t* instance)
{

    FREE_ARRAY(byte, instance->code, instance->capacity);
    init_chunk(instance);
}

void write_chunk(chunk_t* instance, const byte data)
{
    const size_t capacity = instance->capacity;
    if (capacity < instance->count + 1)
    {
        instance->capacity = grow_capacity(capacity);

        instance->code =
            GROW_ARRAY(byte, instance->code, capacity, instance->capacity);
    }

    instance->code[instance->count] = data;
    ++instance->count;
}
