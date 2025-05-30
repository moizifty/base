#ifndef BASE_BITSTREAM_H
#define BASE_BITSTREAM_H

#include "base\baseCore.h"

typedef struct BaseBitstream
{
    U8Array bytes;
    u64 bitIndex;
}BaseBitstream;

#define baseBitstreamPopBitsAsU8(stream, n, out)  baseBitstreamPopBitsAsU8Impl(stream, n, out)
#define baseBitstreamPopBitsAsU64(stream, n, out)  baseBitstreamPopBitsAsU64Impl(stream, n, out)
#define baseBitstreamPopBitsReversedAsU8(stream, n, out) baseBitstreamPopBitsReversedAsU8Impl(stream, n, out)
#define baseBitstreamPopBitsReversedAsU64(stream, n, out) baseBitstreamPopBitsReversedAsU64Impl(stream, n, out)

#define baseBitstreamPeekBitsAsU8(stream, n, out)  baseBitstreamPeekBitsAsU8Impl(stream, n, out)
#define baseBitstreamPeekBitsAsU64(stream, n, out)  baseBitstreamPeekBitsAsU64Impl(stream, n, out)
#define baseBitstreamPeekBitsReversedAsU8(stream, n, out)  baseBitstreamPeekBitsReversedAsU8Impl(stream, n, out)
#define baseBitstreamPeekBitsReversedAsU64(stream, n, out)  baseBitstreamPeekBitsReversedAsU64Impl(stream, n, out)

bool baseBitstreamPopBit(BaseBitstream *stream, u8 *out);
bool baseBitstreamPopBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPopBitsReversedAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPopBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
bool baseBitstreamPopBitsReversedAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);

bool baseBitstreamPopU8(BaseBitstream *stream, u8 *out);
bool baseBitstreamPopU16LE(BaseBitstream *stream, u16 *out);
bool baseBitstreamPopU16BE(BaseBitstream *stream, u16 *out);
bool baseBitstreamPopU32LE(BaseBitstream *stream, u32 *out);
bool baseBitstreamPopU32BE(BaseBitstream *stream, u32 *out);
bool baseBitstreamPopU64LE(BaseBitstream *stream, u64 *out);
bool baseBitstreamPopU64BE(BaseBitstream *stream, u64 *out);

void baseBitstreamPopTillNextByte(BaseBitstream *stream);

void baseBitstreamConsumeBits(BaseBitstream *stream, u64 n);
bool baseBitstreamPeekBit(BaseBitstream *stream, u8 *out);
bool baseBitstreamPeekBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPeekBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
bool baseBitstreamPeekBitsReversedAsU8Impl(BaseBitstream *stream, u8 n, u8 *out);
bool baseBitstreamPeekBitsReversedAsU64Impl(BaseBitstream *stream, u64 n, u64 *out);
#endif