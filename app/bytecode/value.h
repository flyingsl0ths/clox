#pragma once

#include <common.h>
#include <utils/array.h>

typedef double value_t;

ARRAY(value_t, value);

void init_value_array(value_array_t* const instance);
void append_value_array(value_array_t* const instance, value_t value);
void free_value_array(value_array_t* const instance);
void print_value(value_t instance);
