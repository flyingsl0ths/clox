#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include <utils/files.h>
#include <vm/vm.h>

static void repl(vm_t* const vm)
{
    char prompt[2056];
    while (true)
    {
        printf("> ");

        if (!fgets(prompt, sizeof(prompt), stdin)) { break; }

        if (prompt[0] == '\n') { continue; }

        interpret(vm, prompt);
        puts("");
    }
}

static void run_file(vm_t* const vm, str path)
{
    char* const source = read_file(path);

    if (!source)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    const interpret_result_t result = interpret(vm, source);
    puts("");

    free(source);

    if (result == INTERPRET_COMPILE_ERROR) { exit(65); }
    if (result == INTERPRET_RUNTIME_ERROR) { exit(70); }
}

s32 main(const s32 argc, const char** const argv)
{
    vm_t vm = init_vm();

    if (argc == 1) { repl(&vm); }
    else if (argc == 2) { run_file(&vm, argv[1]); }
    else
    {
        fprintf(stderr, "Usage: clox [path]");
        exit(64);
    }

    free_vm(&vm);
    return 0;
}
