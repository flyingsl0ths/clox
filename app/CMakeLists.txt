add_executable(${PROJECT_NAME})

list(
  APPEND
  BYTECODE
  "bytecode/chunk.h"
  "bytecode/chunk.c"
  "bytecode/value.h"
  "bytecode/value.c"
  "bytecode/object.h"
  "bytecode/object.c")

list(APPEND VM "vm/vm.h" "vm/vm.c")

list(
  APPEND
  UTILS
  "utils/mem.h"
  "utils/mem.c"
  "utils/array.h"
  "utils/files.h"
  "utils/files.c"
  "utils/table.h"
  "utils/table.c")

list(APPEND DBG "bytecode/debug/debug.h" "bytecode/debug/debug.c")

list(APPEND SCANNER "scanner/scanner.h" "scanner/scanner.c" "scanner/token.h")

list(APPEND COMPILER "compiler/compiler.h" "compiler/compiler.c")

target_sources(
  ${PROJECT_NAME}
  PRIVATE main.c
          common.h
          ${BYTECODE}
          ${VM}
          ${UTILS}
          ${DBG}
          ${SCANNER}
          ${COMPILER})

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -Wconversion
          -Werror=return-type
          -Wunused-local-typedefs
          $<$<NOT:$<CONFIG:Debug>>:-Werror> # warnings as errors if not Debug
)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/app")

target_compile_definitions(${PROJECT_NAME}
                           PRIVATE $<$<CONFIG:Debug>:DEBUG_TRACE_EXECUTION>)
