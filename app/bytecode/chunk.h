#pragma once

#include <common.h>

typedef enum
    : byte
{
    OP_RETURN
} opcode_t;

typedef struct
{
    size_t count;
    size_t capacity;
    byte*  code;
} chunk_t;

void init_chunk(chunk_t* instance);
void free_chunk(chunk_t* instance);
void write_chunk(chunk_t* instance, byte data);
