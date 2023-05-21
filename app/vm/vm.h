#pragma once

#include <bytecode/chunk.h>

typedef struct
{
    chunk_t* chunk;
} vm_t;

vm_t init_vm();
void free_vm();
