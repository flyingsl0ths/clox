#include <stdio.h>

#include "bytecode/chunk.h"
#include "bytecode/debug/debug.h"
#include "common.h"
#include "vm/vm.h"

int main()
{
    vm_t vm = init_vm();

    chunk_t chunk;
    init_chunk(&chunk);

    size_t constant_added = add_constant(&chunk, 1.2);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant_added, 123);

    write_chunk(&chunk, OP_RETURN, 123);

    disassemble_chunk(&chunk, "Test chunk");

    free_chunk(&chunk);

    return 0;
}
