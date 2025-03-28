#include <stdio.h>
#include <stdlib.h>

#include "files.h"

char* read_file(str path)
{
    FILE* const file = fopen(path, "rb");

    if (!file) { return NULL; }

    fseek(file, 0L, SEEK_END);

    const usize file_size = (usize)ftell(file);

    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);

    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    const usize bytes_read = fread(buffer, sizeof(char), file_size, file);

    if (bytes_read < file_size)
    {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';

    fclose(file);

    return buffer;
}
