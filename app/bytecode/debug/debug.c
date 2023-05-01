#include <stdio.h>

#include "debug.h"

size_t simple_instruction(const char* const name, const size_t offset)
{
    printf("%s", name);
    puts("");
    return offset + 1UL;
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

    const byte instruction = chunk->code[offset];

    switch (instruction)
    {
        case OP_RETURN: return simple_instruction("OP_RETURN", offset);
        default: printf("Unknown opcode %zu", offset); return offset + 1UL;
    }

    return 0UL;
}
