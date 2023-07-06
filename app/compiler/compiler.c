#include <stdio.h>

#include "compiler.h"
#include <scanner/scanner.h>

void compile(str source)
{
    scanner_t scanner = init_scanner(source);

    size_t line = 0UL;

    while (true)
    {
        const token_t token = scan_token(&scanner);

        if (token.line != line)
        {
            printf("%4zu", token.line);
            line = token.line;
        }
        else { printf("   | "); }

        printf("%2d '%.*s'", token.type, token.length, token.start);
        puts("");

        if (token.type == TOKEN_EOF) { break; }
    }
}
