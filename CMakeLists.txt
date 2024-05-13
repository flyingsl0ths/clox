cmake_minimum_required(VERSION 3.26.3 FATAL_ERROR)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug) # ensure CMAKE_BUILD_TYPE is not empty
endif()

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In Source Builds Not Allowed")
endif()

set(NAME "lox")
set(DESCRIPTION "The lox programming language")

project(
  ${NAME}
  VERSION 1.0.0
  DESCRIPTION ${DESCRIPTION}
  LANGUAGES C)

add_library(compile-options INTERFACE)

target_compile_features(compile-options INTERFACE c_std_11)

target_compile_options(
  compile-options INTERFACE "-Wextra;-Wall;-Werror;-Wunused-local-typedefs")

add_subdirectory(app)
