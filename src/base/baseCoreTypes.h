#ifndef BASE_CORE_TYPES_H
#define BASE_CORE_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "baseCtxCrack.h"

#define null NULL

#if COMPILER_MSVC
# define threadlocal __declspec(thread)
#elif COMPILER_CLANG
# define threadlocal __thread
#elif COMPILER_GCC
# define threadlocal __thread
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef struct ArrayView
{
    void *data;
    u64 len;
    u64 elemSize;
}ArrayView;

typedef struct str8
{
    u8 *data;
    u64 len;
}str8;

typedef struct str16
{
    u16 *data;
    u64 len;
}str16;

typedef struct str32
{
    u32 *data;
    u64 len;
}str32;

#endif
