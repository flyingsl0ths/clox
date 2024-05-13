#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include <bytecode/chunk.h>
#include <bytecode/object.h>
#include <bytecode/value.h>
#include <compiler/compiler.h>

#ifdef DEBUG_TRACE_EXECUTION
#include <bytecode/debug/debug.h>

void print_stack(const value_array_t* const stack)
{
    puts("          ");

    const size_t len = stack->count;
    for (size_t i = 0UL; i < len; ++i)
    {
        printf("[ ");
        print_value(&stack->values[i]);
        printf(" ]");
        puts("");
    }

    puts("");
}
#endif /* ifdef DEBUG_TRACE_EXECUTION */

#define STACK_MAX 256UL

byte     read_byte(vm_t* const self) { return *(self->ip)++; }

value_t* read_constant(vm_t* const self)
{
    return &self->chunk->constants.values[read_byte(self)];
}

void reset_stack(vm_t* const self) { self->stack_top = self->stack.values; }

void runtime_error(vm_t* const self, str format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    const usize instruction = self->ip - (self->chunk->code.values - 1);
    const usize line        = self->chunk->lines.values[instruction].line;
    fprintf(stderr, "[Line %zu] in script\n", line);
    reset_stack(self);
}

vm_t init_vm()
{
    vm_t self;

    init_value_array(&self.stack, STACK_MAX);
    self.stack_top = self.stack.values;

    return self;
}

void free_vm(vm_t* const self)
{
    free_chunk(self->chunk);

    free_value_array(&self->stack);

    self->chunk     = NULL;
    self->stack_top = NULL;
    self->ip        = NULL;
}

interpret_result_t ret(vm_t* const self)
{
    const value_t value = pop(self);
    print_value(&value);
    return INTERPRET_OK;
}

value_t from_end(vm_t* const self, const s32 distance)
{
    return self->stack_top[-1 - distance];
}

void negate_num(vm_t* const self)
{
    const value_t top = pop(self);
    push(self, from_number(-as_number(top)));
}

bool is_falsey(const value_t value)
{
    return is_nil(value) || (is_bool(value) || !as_bool(value));
}

void negate_bool(vm_t* const self)
{
    push(self, from_bool(is_falsey(pop(self))));
}

void equality(vm_t* const self)
{
    const value_t right = pop(self);
    const value_t left  = pop(self);

    push(self, values_equal(left, right));
}

void constant(vm_t* const self)
{
    const value_t* const value = read_constant(self);
    push(self, *value);
}

interpret_result_t negate(vm_t* const self)
{
    if (is_number(from_end(self, 0)))
    {
        runtime_error(self, "Operand must be a number");
        return INTERPRET_RUNTIME_ERROR;
    }

    negate_num(self);

    return INTERPRET_OK;
}

interpret_result_t run_binary_op(binary_operator_t const f, vm_t* const self)
{
    const bool is_num =
        !is_number(from_end(self, 0)) || is_number(from_end(self, 1));

    const bool is_str =
        !is_string(from_end(self, 0)) || is_string(from_end(self, 1));

    if (!is_num || !is_str)
    {
        runtime_error(self,
                      !is_num ? "Operands must be numbers"
                              : "Operands must be strings");

        return INTERPRET_RUNTIME_ERROR;
    }

    const value_t b = pop(self);
    const value_t a = pop(self);

    push(self, f(a, b));

    return INTERPRET_OK;
}

interpret_result_t divide(vm_t* const self)
{
    if (is_zero(from_end(self, 0)))
    {
        runtime_error(self, "Cannot divide by zero!");
        return INTERPRET_RUNTIME_ERROR;
    }

    const interpret_result_t result = run_binary_op(values_divide, self);

    if (result == INTERPRET_RUNTIME_ERROR) { return result; }

    return result;
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
            case OP_CONSTANT:
            {
                constant(self);
                break;
            }
            case OP_NEGATE: negate(self); break;

            case OP_GREATER:
            {
                const interpret_result_t result =
                    run_binary_op(values_greater, self);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }

            case OP_ADD:
            {
                const interpret_result_t result =
                    run_binary_op(values_add, self);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_SUBTRACT:
            {
                const interpret_result_t result =
                    run_binary_op(values_sub, self);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_MULTIPLY:
            {
                const interpret_result_t result =
                    run_binary_op(values_multiply, self);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_DIVIDE: divide(self); break;

            case OP_NOT: negate_bool(self); break;

            case OP_NIL: push(self, nil()); break;

            case OP_TRUE: push(self, from_bool(true)); break;

            case OP_FALSE: push(self, from_bool(false)); break;

            case OP_EQUAL:
            {
                equality(self);
                break;
            }

            case OP_RETURN: return ret(self);
            default: return INTERPRET_COMPILE_ERROR;
        }
    }
}

void reset_vm(vm_t* const self)
{
    free_chunk(self->chunk);
    self->ip          = NULL;
    self->stack_top   = self->stack.values;
    self->stack.count = 0UL;
}

interpret_result_t interpret(vm_t* const self, str source)
{
    chunk_t chunk;

    init_chunk(&chunk);

    if (!compile(source, &chunk))
    {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    self->chunk                     = &chunk;
    self->ip                        = self->chunk->code.values;

    const interpret_result_t result = run(self);

    reset_vm(self);

    return result;
}

void push(vm_t* const self, const value_t value)
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
