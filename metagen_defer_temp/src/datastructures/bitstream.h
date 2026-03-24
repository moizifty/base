#ifndef BASE_BITSTREAM_H
#define BASE_BITSTREAM_H

#include "base/baseCore.h"

typedef struct Bitstream
{
    U8Array bytes;
    u64 bitIndex;
}Bitstream;

#define bitstreamPopBitsAsU8(stream, n, out)  bitstreamPopBitsAsU8Impl(stream, n, out)
#define bitstreamPopBitsAsU64(stream, n, out)  bitstreamPopBitsAsU64Impl(stream, n, out)
#define bitstreamPopBitsReversedAsU8(stream, n, out) bitstreamPopBitsReversedAsU8Impl(stream, n, out)
#define bitstreamPopBitsReversedAsU64(stream, n, out) bitstreamPopBitsReversedAsU64Impl(stream, n, out)

#define bitstreamPeekBitsAsU8(stream, n, out)  bitstreamPeekBitsAsU8Impl(stream, n, out)
#define bitstreamPeekBitsAsU64(stream, n, out)  bitstreamPeekBitsAsU64Impl(stream, n, out)
#define bitstreamPeekBitsReversedAsU8(stream, n, out)  bitstreamPeekBitsReversedAsU8Impl(stream, n, out)
#define bitstreamPeekBitsReversedAsU64(stream, n, out)  bitstreamPeekBitsReversedAsU64Impl(stream, n, out)

bool bitstreamPopBit(Bitstream *stream, u8 *out);
bool bitstreamPopBitsAsU8Impl(Bitstream *stream, u8 n, u8 *out);
bool bitstreamPopBitsReversedAsU8Impl(Bitstream *stream, u8 n, u8 *out);
bool bitstreamPopBitsAsU64Impl(Bitstream *stream, u64 n, u64 *out);
bool bitstreamPopBitsReversedAsU64Impl(Bitstream *stream, u64 n, u64 *out);

bool bitstreamPopU8(Bitstream *stream, u8 *out);
bool bitstreamPopU16LE(Bitstream *stream, u16 *out);
bool bitstreamPopU16BE(Bitstream *stream, u16 *out);
bool bitstreamPopU32LE(Bitstream *stream, u32 *out);
bool bitstreamPopU32BE(Bitstream *stream, u32 *out);
bool bitstreamPopU64LE(Bitstream *stream, u64 *out);
bool bitstreamPopU64BE(Bitstream *stream, u64 *out);

void bitstreamPopTillNextByte(Bitstream *stream);

void bitstreamConsumeBits(Bitstream *stream, u64 n);
bool bitstreamPeekBit(Bitstream *stream, u8 *out);
bool bitstreamPeekBitsAsU8Impl(Bitstream *stream, u8 n, u8 *out);
bool bitstreamPeekBitsAsU64Impl(Bitstream *stream, u64 n, u64 *out);
bool bitstreamPeekBitsReversedAsU8Impl(Bitstream *stream, u8 n, u8 *out);
bool bitstreamPeekBitsReversedAsU64Impl(Bitstream *stream, u64 n, u64 *out);
#endif