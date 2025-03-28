#include "table.h"
#include "mem.h"
#include <bytecode/object.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75

ARRAY_INIT(entry, entry_t)

table_t init_table(void)
{
    entry_array_t entries;

    init_entry_array(&entries, 0UL);

    const table_t instance = {.entries = entries};

    return instance;
}

void free_table(table_t* self)
{
    entry_array_t* const entries = &self->entries;

    FREE_ARRAY(entries)
}

static entry_t*
find_entry(entry_t* const entries, const usize capacity, obj_string_t* const key)
{
    u32      index     = key->hash % capacity;

    entry_t* tombstone = NULL;

    while (true)
    {
        entry_t* const entry = &entries[index];
        if (entry->key == NULL)
        {
            if (is_nil(entry->value)) { return tombstone != NULL ? tombstone : entry; }
            if (tombstone == NULL) { tombstone = entry; }
        }
        else if (entry->key == key) { return entry; }

        index = (index + 1) % capacity;
    }
}

static void adjust_capacity(table_t* self, const usize capacity)
{
    entry_t* new_entries = ALLOCATE(entry_t, capacity);

    for (usize i = 0UL; i < capacity; ++i)
    {
        new_entries[i].key   = NULL;
        new_entries[i].value = NIL_VAL;
    }

    self->entries.count = 0UL;

    for (usize i = 0UL, prior_capacity = self->entries.capacity; i < prior_capacity; ++i)
    {
        entry_t* const entry = &self->entries.values[i];
        if (entry->key == NULL) { continue; }

        entry_t* const dest = find_entry(new_entries, capacity, entry->key);
        dest->key           = entry->key;
        dest->value         = entry->value;

        ++self->entries.count;
    }

    entry_array_t* const old_entries = &self->entries;

    FREE_ARRAY(old_entries)

    self->entries.values   = new_entries;
    self->entries.capacity = capacity;
}

bool table_set(table_t* self, obj_string_t* key, value_t value)
{

    entry_array_t* const entries = &self->entries;

    if ((f64)(entries->count + 1) > (f64)entries->capacity * TABLE_MAX_LOAD)
    {
        const usize capacity = grow_capacity(entries->capacity);
        adjust_capacity(self, capacity);
    }

    entry_t* const entry      = find_entry(entries->values, entries->capacity, key);

    const bool     is_new_key = entry->key == NULL;
    if (is_new_key && is_nil(entry->value)) { ++entries->count; }

    entry->key   = key;
    entry->value = value;

    return is_new_key;
}

void table_add_all(table_t* const from, table_t* const to)
{
    for (usize i = 0UL, capacity = from->entries.capacity; i < capacity; ++i)
    {
        entry_t* const entry = &from->entries.values[i];
        if (entry->key != NULL) { table_set(to, entry->key, entry->value); }
    }
}

value_t* table_get(table_t* table, obj_string_t* key)
{
    if (table->entries.count == 0) { return NULL; }

    entry_t* const entry =
        find_entry(table->entries.values, table->entries.capacity, key);

    return entry->key == NULL ? NULL : &entry->value;
}

bool table_delete(table_t* const table, obj_string_t* const key)
{
    if (table->entries.count == 0) { return false; }

    entry_t* const entry =
        find_entry(table->entries.values, table->entries.capacity, key);

    if (entry->key == NULL) { return false; }

    entry->key   = NULL;
    entry->value = from_bool(true);

    return true;
}

obj_string_t*
table_find_string(table_t* const table, str chars, const usize length, const u32 hash)
{
    if (table->entries.count == 0) { return NULL; }

    u32 index = hash % table->entries.capacity;

    while (true)
    {
        const entry_t* const entry = &table->entries.values[index];

        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry.
            if (is_nil(entry->value)) { return NULL; }
        }
        else if (entry->key->length == length && entry->key->hash == hash &&
                 memcmp(entry->key->chars, chars, length) == 0)
        {
            // Found it.
            return entry->key;
        }

        index = (index + 1) % table->entries.capacity;
    }
}
