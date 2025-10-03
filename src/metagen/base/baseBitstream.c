#include "base\baseBitstream.h"

bool bitstreamPopBit(Bitstream *stream, u8 *out)
{
    bool result = bitstreamPeekBit(stream, out);
    bitstreamConsumeBits(stream, 1);

    return result;
}
bool bitstreamPopBitsAsU8Impl(Bitstream *stream, u8 n, u8 *out)
{
    *out = 0;

    for(u64 i = 0; i < n; i++)
    {
        u8 b;
        if (!bitstreamPopBit(stream, &b))
        {
            return false;
        }

        *out = (*out) | ((u8)b << i);
    }

    return true;
}
bool bitstreamPopBitsReversedAsU8Impl(Bitstream *stream, u8 n, u8 *out)
{
    bool r = bitstreamPeekBitsReversedAsU8(stream, n, out);
    bitstreamConsumeBits(stream, n);

    return r;
}
bool bitstreamPopBitsReversedAsU64Impl(Bitstream *stream, u64 n, u64 *out)
{
    bool r = bitstreamPeekBitsReversedAsU64(stream, n, out);
    bitstreamConsumeBits(stream, n);

    return r;
}

bool bitstreamPopBitsAsU64Impl(Bitstream *stream, u64 n, u64 *out)
{
    *out = 0;
    for(u64 i = 0; i < n; i++)
    {
        u8 b;
        if (!bitstreamPopBit(stream, &b))
        {
            return false;
        }

        *out = (*out) | ((u64)b << i);
    }

    return true;
}

bool bitstreamPopU8(Bitstream *stream, u8 *out)
{
    u8 b1 = 0;
    if(!bitstreamPopBitsAsU8(stream, 8, &b1))
    {
        return false;
    }

    *out = (u8)b1;

    return true;
}
bool bitstreamPopU16LE(Bitstream *stream, u16 *out)
{
    u64 b1, b2;
    if(!bitstreamPopBitsAsU64(stream, 8, &b1) ||
       !bitstreamPopBitsAsU64(stream, 8, &b2))
    {
        return false;
    }

    *out = (u16)(b1 | (b2 << 8));

    return true;
}
bool bitstreamPopU16BE(Bitstream *stream, u16 *out)
{
    u64 b1, b2;
    if(!bitstreamPopBitsAsU64(stream, 8, &b1) ||
       !bitstreamPopBitsAsU64(stream, 8, &b2))
    {
        return false;
    }

    *out = (u16)(b2 | (b1 << 8));

    return true;
}
bool bitstreamPopU32LE(Bitstream *stream, u32 *out)
{
    u16 s1, s2;
    if(!bitstreamPopU16LE(stream, &s1) ||
       !bitstreamPopU16LE(stream, &s2))
    {
        return false;
    }

    *out = (u32)((u32)s1 | ((u32)s2 << 16));

    return true;
}
bool bitstreamPopU32BE(Bitstream *stream, u32 *out)
{
    u16 s1, s2;
    if(!bitstreamPopU16BE(stream, &s1) ||
       !bitstreamPopU16BE(stream, &s2))
    {
        return false;
    }

    *out = (u32)((u32)s2 | ((u32)s1 << 16));

    return true;
}
bool bitstreamPopU64LE(Bitstream *stream, u64 *out)
{
    u32 w1, w2;
    if(!bitstreamPopU32LE(stream, &w1) ||
       !bitstreamPopU32LE(stream, &w2))
    {
        return false;
    }

    *out = (u64)((u64)w1 | ((u64)w2 << 32));

    return true;
}
bool bitstreamPopU64BE(Bitstream *stream, u64 *out)
{
    u32 w1, w2;
    if(!bitstreamPopU32BE(stream, &w1) ||
       !bitstreamPopU32BE(stream, &w2))
    {
        return false;
    }

    *out = (u64)((u64)w2 | ((u64)w1 << 32));

    return true;
}

void bitstreamPopTillNextByte(Bitstream *stream)
{
    while(stream->bitIndex % 8 != 0)
    {
        bitstreamConsumeBits(stream, 1);
    }
}

void bitstreamConsumeBits(Bitstream *stream, u64 n)
{
    stream->bitIndex += n;
}
bool bitstreamPeekBit(Bitstream *stream, u8 *out)
{
    u64 bitIndex = stream->bitIndex;

    u64 byteToUse = (bitIndex / 8);
    u64 bitToUse = (bitIndex % 8);

    if (byteToUse >= stream->bytes.len)
    {
        return false;
    }

    *out = (stream->bytes.data[byteToUse] & (1 << bitToUse)) >> bitToUse;

    return true;
}
bool bitstreamPeekBitsAsU8Impl(Bitstream *stream, u8 n, u8 *out)
{
    bool result = true;

    *out = 0;

    u64 i;
    for(i = 0; i < n; i++)
    {
        u8 b;
        if (!bitstreamPeekBit(stream, &b))
        {
            result = false;
            break;
        }

        *out = (*out) | ((u8)b << i);
        stream->bitIndex += 1;
    }

    stream->bitIndex -= i;
    return result;
}
bool bitstreamPeekBitsAsU64Impl(Bitstream *stream, u64 n, u64 *out)
{
    bool result = true;

    *out = 0;

    u64 i;
    for(i = 0; i < n; i++)
    {
        u8 b;
        if (!bitstreamPeekBit(stream, &b))
        {
            result = false;
            break;
        }

        *out = (*out) | ((u64)b << i);
        stream->bitIndex += 1;
    }

    stream->bitIndex -= i;
    return result;
}

bool bitstreamPeekBitsReversedAsU8Impl(Bitstream *stream, u8 n, u8 *out)
{
    u8 code = 0;
    if(bitstreamPeekBitsAsU8Impl(stream, n, &code))
    {
        *out = 0;
        for(u64 i = 0; i < n; i++)
        {
            u8 bit = (code >> i) & 0b1;
            u64 shiftAmount = n - (i + 1);
            *out |= (u8)(bit << shiftAmount);
        }

        return true;
    }

    return false;
}
bool bitstreamPeekBitsReversedAsU64Impl(Bitstream *stream, u64 n, u64 *out)
{
    u64 code = 0;
    if(bitstreamPeekBitsAsU64Impl(stream, n, &code))
    {
        *out = 0;
        for(u64 i = 0; i < n; i++)
        {
            u8 bit = (code >> i) & 0b1;
            u64 shiftAmount = n - (i + 1);
            *out |= (bit << shiftAmount);
        }

        return true;
    }

    return false;
}