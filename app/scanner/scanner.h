#pragma once

#include "token.h"
#include <common.h>

typedef struct
{
    str   start;
    str   current;
    usize line;
} scanner_t;

scanner_t init_scanner(str source);
token_t   scan_token(scanner_t* self);
