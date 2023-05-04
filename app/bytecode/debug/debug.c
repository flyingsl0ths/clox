#include <stdio.h>

#include "debug.h"

size_t simple_instruction(const char* const name, const size_t offset)
{
    printf("%s", name);
    puts("");
    return offset + 1UL;
}

size_t constant_instruction(const char* const    name,
                            const chunk_t* const chunk,
                            const size_t         offset)
{
    byte constant = chunk->code[offset + 1UL];
    printf("%-16s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    puts("'");
    return offset + 2UL;
}

void disassemble_chunk(chunk_t* const chunk, const char* const name)
{
    printf("== %s ==", name);
    puts("");

    for (size_t offset = 0UL; offset < chunk->count;)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

size_t disassemble_instruction(chunk_t* const chunk, const size_t offset)
{
    printf("%04zu", offset);

    if (offset > 0UL && chunk->lines[offset] == chunk->lines[offset - 1UL])
    {
        puts("   | ");
    }
    else { printf("%4d ", chunk->lines[offset]); }

    const byte instruction = chunk->code[offset];

    switch (instruction)
    {
        case OP_RETURN: return simple_instruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        default: printf("Unknown opcode %zu", offset); return offset + 1UL;
    }

    return 0UL;
}
