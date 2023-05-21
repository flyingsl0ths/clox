#include "vm.h"
#include <stdlib.h>

vm_t init_vm()
{
    vm_t vm;
    init_chunk(vm.chunk);

    return vm;
}
