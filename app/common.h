#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t      byte;
typedef int          s32;
typedef float        f32;
typedef double       f64;
typedef unsigned int u32;
typedef const char*  str;

#define INT(n)   n
#define USZ(n)   n##UL
#define FL(n, d) ((f32)n##.##d)
#define DB(n, d) n##.##d
