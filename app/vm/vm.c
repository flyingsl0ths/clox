#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include <bytecode/value.h>

#ifdef DEBUG_TRACE_EXECUTION
#include <bytecode/debug/debug.h>

void print_stack(value_t* const stack, value_t* const stack_top)
{
    puts("          ");
    for (value_t const* slot = stack; slot < stack_top; ++slot)
    {
        puts("[ ");
        print_value(*slot);
        puts(" ]");
    }
    puts("");
}
#endif /* ifdef DEBUG_TRACE_EXECUTION */

#define STACK_MAX 256UL

byte read_byte(vm_t* instance) { return *(instance->ip)++; }

value_t read_constant(vm_t* instance)
{
    return instance->chunk->constants.values[read_byte(instance)];
}

vm_t reset_stack(vm_t instance)
{
    instance.stack_top = instance.stack.values;
    return instance;
}

vm_t init_vm()
{
    vm_t vm;

    init_chunk(vm.chunk);

    vm.stack.values   = (value_t*)malloc(sizeof(value_t) * STACK_MAX);
    vm.stack.capacity = STACK_MAX;

    return vm;
}

void free_vm(vm_t* instance) {}

interpret_result_t retrn(vm_t* const instance)
{
    const value_t value = pop(instance);
    print_value(value);
    return INTERPRET_OK;
}

void constant(vm_t* const instance)
{
    const value_t value = read_constant(instance);
    push(instance, value);
    puts("");
}

interpret_result_t run(vm_t* const instance)
{
    read_byte(instance);

    while (true)
    {

#ifdef DEBUG_TRACE_EXECUTION
        print_stack(instance->stack.values, instance->stack_top);

        disassemble_instruction(
            instance->chunk,
            (size_t)(instance->ip - instance->chunk->code.values));
#endif /* ifdef DEBUG_TRACE_EXECUTION */

        const byte instruction = read_byte(instance);

        switch (instruction)
        {
            case OP_RETURN: retrn(instance);
            case OP_CONSTANT:
            {
                constant(instance);
                break;
            }
        }
    }
}

interpret_result_t interpret(vm_t* const instance, chunk_t* code)
{
    instance->chunk = code;
    code            = NULL;
    instance->ip    = instance->chunk->code.values;

    return INTERPRET_OK;
}

void push(vm_t* const instance, value_t value)
{
    *instance->stack_top = value;
    ++instance->stack_top;
}

value_t pop(vm_t* const instance)
{
    --instance->stack_top;
    return *instance->stack_top;
}
