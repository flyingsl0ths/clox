#pragma once

#include <bytecode/chunk.h>

void  disassemble_chunk(chunk_t* chunk, const char* name);

usize disassemble_instruction(chunk_t* chunk, usize offset);
