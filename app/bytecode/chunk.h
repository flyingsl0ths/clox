#pragma once

#include "value.h"
#include <common.h>
#include <utils/array.h>

typedef enum : byte
{
    OP_CONSTANT,
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

void   init_chunk(chunk_t* instance);
void   free_chunk(chunk_t* instance);
void   write_chunk(chunk_t* instance, byte data, u32 line);
size_t add_constant(chunk_t* instance, value_t value);
