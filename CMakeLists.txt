cmake_minimum_required(VERSION 3.26.3 FATAL_ERROR)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(NAME "lox")
set(DESCRIPTION "The lox programming language")

project(
  ${NAME}
  VERSION 1.0.0
  DESCRIPTION ${DESCRIPTION}
  LANGUAGES C)

add_subdirectory(app)
