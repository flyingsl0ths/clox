#pragma once

#include <common.h>
#include <utils/array.h>

typedef double value_t;

ARRAY(value_t, value);

void init_value_array(value_array_t* const self);
void append_value_array(value_array_t* const self, value_t value);
void free_value_array(value_array_t* const self);
void print_value(value_t self);

value_t add_values(value_t left, value_t right);
value_t sub_values(value_t left, value_t right);
value_t mul_values(value_t left, value_t right);
value_t div_values(value_t left, value_t right);
