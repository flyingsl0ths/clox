#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/table.h"
#include "vm.h"
#include <bytecode/object.h>
#include <compiler/compiler.h>

#ifdef DEBUG_TRACE_EXECUTION
#include <bytecode/debug/debug.h>

static void print_stack(const value_array_t* const stack)
{
    puts("          ");

    const usize len = stack->count;
    for (usize i = 0UL; i < len; ++i)
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

static u8       read_byte(vm_t* const self) { return *(self->ip)++; }

static value_t* read_constant(vm_t* const self)
{
    return &self->chunk.constants.values[read_byte(self)];
}

static obj_string_t* read_string(vm_t* const self)
{
    return as_string(*read_constant(self));
}

static void reset_stack(vm_t* const self)
{
    self->stack_top = self->stack.values;
}

static void runtime_error(vm_t* const self, str format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    const usize instruction = self->ip - (self->chunk.code.values - 1);
    const usize line        = self->chunk.lines.values[instruction].line;
    fprintf(stderr, "[Line %zu] in script\n", line);
    reset_stack(self);
}

vm_t init_vm()
{
    vm_t self;

    init_value_array(&self.stack, STACK_MAX);

    self.stack_top = self.stack.values;

    self.objects   = NULL;

    self.globals   = init_table();

    self.strings   = init_table();

    return self;
}

static void free_objects(object_t* objects)
{
    object_t* object = objects;

    while (object != NULL)
    {
        object_t* const next = object->next;
        free_object(object);
        object = next;
    }
}

void free_vm(vm_t* const self)
{
    free_chunk(&self->chunk);

    free_value_array(&self->stack);

    self->stack_top = NULL;
    self->ip        = NULL;

    free_table(&self->globals);

    free_table(&self->strings);

    free_objects(self->objects);
}

static void push(vm_t* const self, const value_t value)
{
    *self->stack_top = value;
    ++self->stack_top;
    ++self->stack.count;
}

static value_t pop(vm_t* const self)
{
    --self->stack_top;
    --self->stack.count;
    return *self->stack_top;
}

static value_t from_end(vm_t* const self, const s32 distance)
{
    return self->stack_top[-1 - distance];
}

static value_t peek(const vm_t* const self) { return self->stack_top[-1 - 0]; }

static void    define_global(vm_t* const self)
{
    obj_string_t* const name = read_string(self);
    table_set(&self->globals, name, peek(self));
    pop(self);
}

static void negate_num(vm_t* const self)
{
    const value_t top = pop(self);
    push(self, from_number(-as_number(top)));
}

static bool is_falsey(const value_t value)
{
    return is_nil(value) || (is_bool(value) || !as_bool(value));
}

static void negate_bool(vm_t* const self)
{
    push(self, from_bool(is_falsey(pop(self))));
}

static void equality(vm_t* const self)
{
    const value_t right = pop(self);
    const value_t left  = pop(self);

    push(self, values_equal(left, right));
}

static void constant(vm_t* const self)
{
    const value_t* const value = read_constant(self);
    push(self, *value);
}

static interpret_result_t negate(vm_t* const self)
{
    if (is_number(peek(self)))
    {
        runtime_error(self, "Operand must be a number");
        return INTERPRET_RUNTIME_ERROR;
    }

    negate_num(self);

    return INTERPRET_OK;
}

static void print_stmt(vm_t* const self)
{
    const value_t value = pop(self);
    print_value(&value);
    puts("");
}

static interpret_result_t ensure_types(vm_t* const        self,
                                       const value_type_t a,
                                       const value_type_t b,
                                       str                messages[2])
{
    const value_t left      = peek(self);
    const value_t right     = from_end(self, 1);

    const bool    is_first  = left.type == a && right.type == a;
    const bool    is_second = left.type == b && right.type == b;

    if (!(is_first || is_second))
    {
        runtime_error(self, is_first ? messages[0] : messages[1]);
        return INTERPRET_RUNTIME_ERROR;
    }

    return INTERPRET_OK;
}

static interpret_result_t run_add_op(vm_t* const self)
{
    if (ensure_types(
            self,
            VAL_NUM,
            VAL_OBJ,
            (str[]){"Operands must be numbers", "Operands must be strings"}))
    {
        return INTERPRET_RUNTIME_ERROR;
    }

    const value_t b = pop(self);
    const value_t a = pop(self);

    push(self, values_add(a, b, self->objects, &self->strings));

    return INTERPRET_OK;
}

static interpret_result_t run_binary_op(vm_t* const             self,
                                        binary_operator_t const f)
{
    if (ensure_types(
            self,
            VAL_NUM,
            VAL_OBJ,
            (str[]){"Operands must be numbers", "Operands must be strings"}))
    {
        return INTERPRET_RUNTIME_ERROR;
    }

    const value_t b = pop(self);
    const value_t a = pop(self);

    push(self, f(a, b));

    return INTERPRET_OK;
}

static interpret_result_t divide(vm_t* const self)
{
    if (is_zero(peek(self)))
    {
        runtime_error(self, "Cannot divide by zero!");
        return INTERPRET_RUNTIME_ERROR;
    }

    return run_binary_op(self, values_divide);
}

static interpret_result_t run(vm_t* const self)
{
    while (true)
    {

#ifdef DEBUG_TRACE_EXECUTION
        print_stack(&self->stack);

        disassemble_instruction(&self->chunk,
                                (usize)(self->ip - self->chunk.code.values));
#endif /* ifdef DEBUG_TRACE_EXECUTION */

        const u8 instruction = read_byte(self);

        switch (instruction)
        {
            case OP_CONSTANT:
            {
                constant(self);
                break;
            }
            case OP_NEGATE: negate(self); break;

            case OP_PRINT: print_stmt(self); break;

            case OP_GREATER:
            {
                const interpret_result_t result =
                    run_binary_op(self, values_greater);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }

            case OP_LESS:
            {
                const interpret_result_t result =
                    run_binary_op(self, values_less);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }

            case OP_ADD:
            {
                const interpret_result_t result = run_add_op(self);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_SUBTRACT:
            {
                const interpret_result_t result =
                    run_binary_op(self, values_sub);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_MULTIPLY:
            {
                const interpret_result_t result =
                    run_binary_op(self, values_multiply);
                if (result == INTERPRET_RUNTIME_ERROR) { return result; }
                break;
            }
            case OP_DIVIDE: divide(self); break;

            case OP_NOT: negate_bool(self); break;

            case OP_NIL: push(self, nil()); break;

            case OP_TRUE: push(self, from_bool(true)); break;

            case OP_FALSE: push(self, from_bool(false)); break;

            case OP_POP: pop(self); break;

            case OP_DEFINE_GLOBAL: define_global(self); break;

            case OP_GET_GLOBAL:
            {

                obj_string_t* const name  = read_string(self);
                value_t* const      value = table_get(&self->globals, name);

                if (!value)
                {
                    runtime_error(self, "Undefined variable '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(self, *value);
                break;
            }

            case OP_SET_GLOBAL:
            {
                obj_string_t* const name    = read_string(self);
                table_t* const      globals = &self->globals;

                if (table_set(globals, name, peek(self)))
                {
                    table_delete(globals, name);
                    runtime_error(self, "Undefined variable '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_EQUAL:
            {
                equality(self);
                break;
            }

            case OP_RETURN: return INTERPRET_OK;
            default: return INTERPRET_COMPILE_ERROR;
        }
    }
}

static void reset_vm(vm_t* const self)
{
    free_chunk(&self->chunk);
    self->ip          = NULL;
    self->stack_top   = self->stack.values;
    self->stack.count = 0UL;
}

interpret_result_t interpret(vm_t* const self, str source)
{
    chunk_t chunk;

    init_chunk(&chunk);

    if (!compile(source, &chunk, self->objects, &self->strings))
    {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    self->chunk                     = chunk;
    self->ip                        = self->chunk.code.values;

    const interpret_result_t result = run(self);

    reset_vm(self);

    return result;
}
