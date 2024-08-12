#include "chunk.h"
#include <utils/mem.h>

ARRAY_INIT(line, line_start_t)

void init_chunk(chunk_t* const self)
{
    self->code.count    = 0;
    self->code.capacity = 0;
    self->code.values   = NULL;

    init_line_array(&self->lines, 0);

    init_value_array(&self->constants, 0);
}

void free_chunk(chunk_t* const self)
{
    chunk_array_t* const code  = &self->code;
    line_array_t* const  lines = &self->lines;

    FREE_ARRAY(code);
    FREE_ARRAY(lines);

    free_value_array(&self->constants);
}

bool was_initialized(chunk_t* const self)
{
    return self->code.values == NULL && self->lines.values == NULL &&
           self->constants.values == NULL;
}

void write_chunk(chunk_t* const self, const byte data, const u32 line)
{
    if (self->code.capacity < self->code.count + 1)
    {
        const size_t old_capacity = self->code.capacity;

        self->code.capacity       = grow_capacity(old_capacity);

        self->code.values =
            GROW_ARRAY(byte, self->code.values, self->code.capacity);
    }

    self->code.values[self->code.count] = data;
    ++self->code.count;

    if (self->lines.count > 0 &&
        self->lines.values[self->lines.count - 1].line == line)
    {
        return;
    }

    if (self->lines.capacity < self->lines.count + 1)
    {
        const size_t old_capacity = self->lines.count;
        self->lines.capacity      = grow_capacity(old_capacity);

        self->lines.values =
            GROW_ARRAY(line_start_t, self->lines.values, self->lines.capacity);
    }

    const size_t        line_count = self->lines.count++;

    line_start_t* const line_start = &(self->lines.values[line_count]);

    line_start->offset             = self->code.count - 1;
    line_start->line               = line;
}

size_t add_constant(chunk_t* const self, const value_t value)
{
    append_value_array(&self->constants, value);
    return self->constants.count - 1;
}

size_t get_line(chunk_t* const self, const size_t instruction)
{
    size_t start = 0UL;
    size_t end   = self->lines.count - 1UL;

    while (true)
    {
        size_t                    mid  = (start + end) / 2UL;
        line_start_t const* const line = &self->lines.values[mid];

        if (instruction < line->offset) { end = mid - 1UL; }
        else if (mid == self->lines.count - 1UL ||
                 instruction < self->lines.values[mid + 1UL].offset)
        {
            return line->line;
        }
        else { start = mid + 1UL; }
    }
}
