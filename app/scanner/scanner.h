#pragma once

#include "token.h"
#include <common.h>

typedef struct
{
    str    start;
    str    current;
    size_t line;
} scanner_t;

scanner_t init_scanner(str source);
token_t   scan_token(scanner_t* const self);
