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

usize disassemble_instruction(chunk_t* const chunk, const usize offset)
{
    printf("%04zu", offset);

    const usize line = get_line(chunk, offset);
    if (offset > 0UL && line == get_line(chunk, offset - 1UL))
    {
        printf("   | ");
    }
    else { printf("%4zu ", line); }

    const u8 instruction = chunk->code.values[offset];

    switch (instruction)
    {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_NEGATE: return simple_instruction("OP_NEGATE", offset);
        case OP_PRINT: return simple_instruction("OP_PRINT", offset);
        case OP_ADD: return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT: return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY: return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE: return simple_instruction("OP_DIVIDE", offset);
        case OP_NOT: return simple_instruction("OP_NOT", offset);
        case OP_RETURN: return simple_instruction("OP_RETURN", offset);
        case OP_NIL: return simple_instruction("OP_NIL", offset);
        case OP_TRUE: return simple_instruction("OP_TRUE", offset);
        case OP_FALSE: return simple_instruction("OP_FALSE", offset);
        case OP_POP: return simple_instruction("OP_POP", offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_EQUAL: return simple_instruction("OP_EQUAL", offset);
        case OP_GREATER: return simple_instruction("OP_GREATER", offset);
        case OP_LESS: return simple_instruction("OP_LESS", offset);
        default: printf("Unknown opcode %zu", offset); return offset + 1UL;
    }
}
