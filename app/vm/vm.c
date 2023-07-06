#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include <bytecode/chunk.h>
#include <bytecode/value.h>
#include <compiler/compiler.h>

#ifdef DEBUG_TRACE_EXECUTION
#include <bytecode/debug/debug.h>

void print_stack(value_array_t const* const stack)
{
    puts("          ");

    const size_t len = stack->count;
    for (size_t i = 0UL; i < len; ++i)
    {
        printf("[ ");
        print_value(stack->values[i]);
        printf(" ]");
        puts("");
    }

    puts("");
}
#endif /* ifdef DEBUG_TRACE_EXECUTION */

#define STACK_MAX 256UL

byte read_byte(vm_t* self) { return *(self->ip)++; }

value_t* read_constant(vm_t* self)
{
    return &self->chunk->constants.values[read_byte(self)];
}

void reset_stack(vm_t* const self) { self->stack_top = self->stack.values; }

vm_t init_vm()
{
    vm_t self;

    self.stack.values   = (value_t*)malloc(sizeof(value_t) * STACK_MAX);
    self.stack_top      = self.stack.values;
    self.stack.count    = 0UL;
    self.stack.capacity = STACK_MAX;

    return self;
}

void free_vm(vm_t* const self)
{
    self->chunk     = NULL;
    self->stack_top = NULL;
    self->ip        = NULL;

    free_value_array(&self->stack);
}

interpret_result_t ret(vm_t* const self)
{
    value_t value = pop(self);
    print_value(value);
    return INTERPRET_OK;
}

void negate(vm_t* const self)
{
    value_t top = pop(self);
    push(self, -top);
}

void constant(vm_t* const self)
{
    value_t* const value = read_constant(self);
    push(self, *value);
}

typedef value_t (*binary_op_t)(value_t, value_t);

void run_binary_op(binary_op_t const f, vm_t* const self)
{
    const value_t b = pop(self);
    const value_t a = pop(self);

    push(self, f(a, b));
}

interpret_result_t run(vm_t* const self)
{
    while (true)
    {

#ifdef DEBUG_TRACE_EXECUTION
        print_stack(&self->stack);

        disassemble_instruction(self->chunk,
                                (size_t)(self->ip - self->chunk->code.values));
#endif /* ifdef DEBUG_TRACE_EXECUTION */

        const byte instruction = read_byte(self);

        switch (instruction)
        {
            case OP_RETURN: return ret(self);
            case OP_NEGATE:
            {
                negate(self);
                break;
            }
            case OP_ADD:
            {
                run_binary_op(add_values, self);
                break;
            }
            case OP_SUBTRACT:
            {
                run_binary_op(sub_values, self);
                break;
            }
            case OP_MULTIPLY:
            {
                run_binary_op(mul_values, self);
                break;
            }
            case OP_DIVIDE:
            {
                run_binary_op(div_values, self);
                break;
            }
            case OP_CONSTANT:
            {
                constant(self);
                break;
            }
        }
    }
}

interpret_result_t interpret(vm_t* const self, str source)
{
    compile(source);
    return INTERPRET_OK;
}

void push(vm_t* const self, value_t value)
{
    *self->stack_top = value;
    ++self->stack_top;
    ++self->stack.count;
}

value_t pop(vm_t* const self)
{
    --self->stack_top;
    --self->stack.count;
    return *self->stack_top;
}
