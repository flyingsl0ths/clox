#pragma once

#include <bytecode/value.h>
#include <common.h>

typedef struct
{
    obj_string_t* key;
    value_t       value;
} entry_t;

ARRAY(entry_t, entry)

typedef struct
{
    entry_array_t entries;
} table_t;

table_t init_table();

void    free_table(table_t* table);

bool    table_set(table_t* table, obj_string_t* const key, const value_t value);

void    table_add_all(table_t* const from, table_t* const to);

value_t* table_get(table_t* const table, obj_string_t* const key);

bool     table_delete(table_t* const table, obj_string_t* const key);
