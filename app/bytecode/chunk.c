#include "chunk.h"
#include <utils/memory.h>

ARRAY_INIT(line)

void init_chunk(chunk_t* const instance)
{
    instance->code.count    = 0;
    instance->code.capacity = 0;
    instance->code.values   = NULL;

    init_line_array(&instance->lines);

    init_value_array(&instance->constants);
}

void free_chunk(chunk_t* const instance)
{

    FREE_ARRAY(byte, instance->code.values, instance->code.capacity);
    FREE_ARRAY(u32, instance->lines.values, instance->lines.capacity);
    free_value_array(&instance->constants);
    init_chunk(instance);
}

void write_chunk(chunk_t* const instance, const byte data, const u32 line)
{

    if (instance->code.capacity < instance->code.count + 1)
    {
        const size_t old_capacity = instance->code.capacity;

        instance->code.capacity = grow_capacity(old_capacity);

        byte* const code = GROW_ARRAY(byte,
                                      &instance->code.values,
                                      instance->code.capacity,
                                      instance->code.capacity);

        instance->code.values = code;
    }

    instance->code.values[instance->code.count] = data;
    ++instance->code.count;

    if (instance->lines.count > 0 &&
        instance->lines.values[instance->lines.count - 1].line == line)
    {
        return;
    }

    if (instance->lines.capacity < instance->lines.count + 1)
    {
        const size_t old_capacity = instance->lines.count;
        instance->lines.capacity  = grow_capacity(old_capacity);

        line_start_t* const lines = GROW_ARRAY(line_start_t,
                                               instance->lines.values,
                                               old_capacity,
                                               instance->lines.capacity);

        instance->lines.values = lines;
    }

    const size_t line_count = ++instance->lines.count;

    line_start_t* const line_start =
        &(instance->lines.values[line_count - 1UL]);

    line_start->offset = instance->code.count - 1;
    line_start->line   = line;
}

size_t add_constant(chunk_t* const instance, const value_t value)
{
    append_value_array(&instance->constants, value);
    return instance->constants.count - 1;
}
