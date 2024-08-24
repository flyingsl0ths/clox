#pragma once

#include <bytecode/chunk.h>
#include <utils/table.h>

typedef enum : u8
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct
{
    chunk_t        chunk;
    byte*          ip;
    value_t*       stack_top;
    struct table_t strings;
    object_t*      objects;
    value_array_t  stack;
} vm_t;

vm_t               init_vm();
void               free_vm(vm_t* self);
interpret_result_t interpret(vm_t* self, str source);
