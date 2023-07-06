#pragma once

#include "value.h"
#include <common.h>
#include <utils/array.h>

typedef enum : byte
{
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN
} opcode_t;

typedef struct
{
    size_t offset;
    size_t line;
} line_start_t;

ARRAY(byte, chunk);
ARRAY(line_start_t, line);

typedef struct
{
    chunk_array_t code;
    line_array_t  lines;
    value_array_t constants;
} chunk_t;

void   init_chunk(chunk_t* const self);
void   free_chunk(chunk_t* const self);
void   write_chunk(chunk_t* const self, byte data, u32 line);
size_t add_constant(chunk_t* const self, value_t value);
size_t get_line(chunk_t const* const self, size_t instruction);
