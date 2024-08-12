#pragma once

#include <bytecode/value.h>

typedef struct
{
    obj_string_t* key;
    value_t       value;
} entry_t;

ARRAY(entry_t, entry)

struct table_t
{
    entry_array_t entries;
};

table_t init_table();

void    free_table(table_t* table);

bool    table_set(table_t* table, obj_string_t* const key, const value_t value);

void    table_add_all(table_t* const from, table_t* const to);

value_t*      table_get(const table_t* const table, obj_string_t* const key);

bool          table_delete(table_t* const table, obj_string_t* const key);

obj_string_t* table_find_string(table_t* const table,
                                str            chars,
                                const usize    length,
                                const u32      hash);
