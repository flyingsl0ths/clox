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

table_t       init_table(void);

void          free_table(table_t* self);

bool          table_set(table_t* table, obj_string_t* key, value_t value);

void          table_add_all(table_t* from, table_t* to);

value_t*      table_get(table_t* table, obj_string_t* key);

bool          table_delete(table_t* table, obj_string_t* key);

obj_string_t* table_find_string(table_t* table, str chars, usize length, u32 hash);
