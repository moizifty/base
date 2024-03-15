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

typedef struct BaseArrayView
{
    void *data;
    u64 elemSize;
    u64 len;
}BaseArrayView;

typedef struct BaseString8
{
    u8 *data;
    u64 len;
}BaseString8;

typedef struct BaseString16
{
    u16 *data;
    u64 len;
}BaseString16;

typedef struct BaseString32
{
    u32 *data;
    u64 len;
}BaseString32;

#endif
