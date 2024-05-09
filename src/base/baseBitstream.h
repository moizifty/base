#ifndef BASE_BITSTREAM_H
#define BASE_BITSTREAM_H

#include "base\baseCore.h"

typedef struct BaseBitstream
{
    U8Array bytes;
    u64 bitIndex;
}BaseBitstream;

#define baseBitstreamPopBitsAsU8(stream, n, out)  baseBitstreamPopBitsAsU8Impl(stream, n, (u8*)out)
#define baseBitstreamPopBitsAsU64(stream, n, out)  baseBitstreamPopBitsAsU64Impl(stream, n, (u64*)out)

bool baseBitstreamPopBit(BaseBitstream *stream, u8 *out);
bool baseBitstreamPopBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPopBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
bool baseBitstreamPopU16LE(BaseBitstream *stream, u16 *out);

void baseBitstreamConsumeBits(BaseBitstream *stream, u64 n);
bool baseBitstreamPeekBit(BaseBitstream *stream, u8 *out);
bool baseBitstreamPeekBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPeekBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
bool baseBitstreamPeekBitsReversedAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
#endif