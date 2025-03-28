#include <stdio.h>

#include "bytecode/chunk.h"
#include "debug.h"

static usize simple_instruction(const char* const name, const usize offset)
{
    printf("%s", name);
    puts("");
    return offset + 1UL;
}

static usize constant_instruction(const char* const    name,
                                  const chunk_t* const chunk,
                                  const usize          offset)
{
    const u8 constant = chunk->code.values[offset + 1UL];
    printf("%-16s %4d '", name, constant);
    print_value(&chunk->constants.values[constant]);
    puts("'");
    return offset + 2UL;
}

void disassemble_chunk(chunk_t* const chunk, const char* const name)
{
    printf("== %s ==", name);
    puts("");

    for (usize offset = 0UL; offset < chunk->code.count;)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

static usize
byte_instruction(const char* const name, const chunk_t* const chunk, const usize offset)
{
    const u8 slot = chunk->code.values[offset + 1UL];
    printf("%-16s %4d\n", name, slot);
    return offset + 2UL;
}

usize disassemble_instruction(chunk_t* const chunk, const usize offset)
{
    printf("%04zu", offset);

    const usize line = get_line(chunk, offset);
    if (offset > 0UL && line == get_line(chunk, offset - 1UL)) { printf("   | "); }
    else { printf("%4zu ", line); }

    const u8 instruction = chunk->code.values[offset];
    usize    result;

    switch (instruction)
    {
        case OP_CONSTANT: result = constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_NEGATE: result = simple_instruction("OP_NEGATE", offset);
        case OP_PRINT: result = simple_instruction("OP_PRINT", offset);
        case OP_ADD: result = simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT: result = simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY: result = simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE: result = simple_instruction("OP_DIVIDE", offset);
        case OP_NOT: result = simple_instruction("OP_NOT", offset);
        case OP_RETURN: result = simple_instruction("OP_RETURN", offset);
        case OP_NIL: result = simple_instruction("OP_NIL", offset);
        case OP_TRUE: result = simple_instruction("OP_TRUE", offset);
        case OP_FALSE: result = simple_instruction("OP_FALSE", offset);
        case OP_POP: result = simple_instruction("OP_POP", offset);
        case OP_POPN: result = simple_instruction("OP_POPN", offset);
        case OP_DEFINE_GLOBAL:
            result = byte_instruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_GET_LOCAL: result = byte_instruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
        case OP_GET_GLOBAL: result = constant_instruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_GLOBAL: result = constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_EQUAL: result = simple_instruction("OP_EQUAL", offset);
        case OP_GREATER: result = simple_instruction("OP_GREATER", offset);
        case OP_LESS: result = simple_instruction("OP_LESS", offset);
        default: printf("Unknown opcode %zu", offset); result = offset + 1UL;
    }

    return result;
}
