#pragma once

#include "common.h"
#include <bytecode/chunk.h>

bool compile(str source, chunk_t* chunk, object_t* objects, table_t* strings);
