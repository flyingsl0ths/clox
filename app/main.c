#include <stdio.h>

#include "bytecode/chunk.h"
#include "common.h"

int main()
{
    chunk_t chunk;
    init_chunk(&chunk);
    write_chunk(&chunk, OP_RETURN);
    free_chunk(&chunk);

    return 0;
}
