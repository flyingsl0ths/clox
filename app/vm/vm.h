#pragma once

#include <bytecode/chunk.h>
#include <utils/table.h>

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct
{
    chunk_t       chunk;
    u8*           ip;
    value_t*      stack_top;
    table_t       strings;
    table_t       globals;
    object_t*     objects;
    value_array_t stack;
} vm_t;

vm_t               init_vm(void);
void               free_vm(vm_t* self);
interpret_result_t interpret(vm_t* self, str source);
