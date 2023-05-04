#pragma once

#include <common.h>

typedef double value_t;

typedef struct
{
    size_t   capacity;
    size_t   count;
    value_t* values;
} value_array_t;

void init_value_array(value_array_t* instance);
void append_value_array(value_array_t* instance, value_t value);
void free_value_array(value_array_t* instance);
void print_value(value_t instance);
