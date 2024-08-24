#pragma once

#include "value.h"

#define BYTE_MAX 255

typedef enum : u8
{
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_NIL,
    OP_RETURN
} opcode_t;

typedef struct
{
    usize offset;
    usize line;
} line_start_t;

ARRAY(u8, chunk)

ARRAY(line_start_t, line)

typedef struct
{
    chunk_array_t code;
    line_array_t  lines;
    value_array_t constants;
} chunk_t;

void   init_chunk(chunk_t* self);
void   free_chunk(chunk_t* self);
bool   was_initialized(chunk_t* self);
size_t add_constant(chunk_t* self, value_t value);
void  write_chunk(chunk_t* self, u8 data, u32 line);
usize get_line(chunk_t* self, usize instruction);
