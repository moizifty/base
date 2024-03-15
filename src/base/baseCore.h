#ifndef BASE_CORE_H
#define BASE_CORE_H

#include <stdlib.h>
#include <string.h>

#include "baseCoreTypes.h"

// General
#define BASE_BYTES(NUM) ((u64)(NUM))
#define BASE_KILOBYTES(NUM) ((u64)(BASE_BYTES(NUM)) * 1024u)
#define BASE_MEGABYTES(NUM) ((u64)(BASE_KILOBYTES(NUM)) * 1024u)
#define BASE_GIGABYTES(NUM) ((u64)(BASE_MEGABYTES(NUM)) * 1024u)

#define BASE_IS_POWER_OF_2(NUM) ((NUM) & ((NUM) - 1))
#define BASE_NUM_BETWEEN(X, S, E)     (((X) >= (S)) && ((X) <= (E)))
#define BASE_ARRAY_SIZE(ARR)  ((sizeof(ARR)) / (sizeof((ARR[0]))))
#define BASE_MAX(A, B)  (((A) > (B)) ? (A) : (B))

// Bitwise
#define BASE_SET_FLAG(n, f)    ((n) |= (f));

#define BASE_MEMSET memset
#define BASE_MEMZERO(DEST, SZ) BASE_MEMSET((DEST), 0, (SZ))
#endif