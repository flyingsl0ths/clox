add_executable(${PROJECT_NAME} main.c)

list(APPEND BYTECODE "bytecode/chunk.h" "bytecode/chunk.c" "bytecode/value.h"
     "bytecode/value.c")
list(APPEND VM "vm/vm.h" "vm/vm.c")
list(APPEND UTILS "utils/memory.h" "utils/memory.c" "utils/array.h")
list(APPEND DBG "bytecode/debug/debug.h" "bytecode/debug/debug.c")

target_sources(${PROJECT_NAME} PRIVATE common.h "${BYTECODE}" "${VM}"
                                       "${UTILS}" "${DBG}")

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/app")

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
target_compile_definitions(${PROJECT_NAME}
                           PRIVATE $<$<CONFIG:Debug>:DEBUG_TRACE_EXECUTION>)
