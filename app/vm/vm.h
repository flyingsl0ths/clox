#pragma once

#include "common.h"
#include <bytecode/chunk.h>
#include <bytecode/value.h>

typedef enum : byte
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct
{
    chunk_t*      chunk;
    byte*         ip;
    value_t*      stack_top;
    value_array_t stack;
} vm_t;

vm_t               init_vm();
void               free_vm(vm_t* const self);
interpret_result_t interpret(vm_t* const self, str source);
void               push(vm_t* const self, value_t value);
value_t            pop(vm_t* const self);
