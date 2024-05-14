#include "base\baseBitstream.h"

bool baseBitstreamPopBit(BaseBitstream *stream, u8 *out)
{
    bool result = baseBitstreamPeekBit(stream, out);
    baseBitstreamConsumeBits(stream, 1);

    return result;
}
bool baseBitstreamPopBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out)
{
    *out = 0;

    for(u64 i = 0; i < n; i++)
    {
        u8 b;
        if (!baseBitstreamPopBit(stream, &b))
        {
            return false;
        }

        *out = (*out) | ((u8)b << i);
    }

    return true;
}
bool baseBitstreamPopBitsReversedAsU8Impl(BaseBitstream *stream, u8 n, u8 *out)
{
    bool r = baseBitstreamPeekBitsReversedAsU8(stream, n, out);
    baseBitstreamConsumeBits(stream, n);

    return r;
}
bool baseBitstreamPopBitsReversedAsU64Impl(BaseBitstream *stream, u64 n, u64 *out)
{
    bool r = baseBitstreamPeekBitsReversedAsU64(stream, n, out);
    baseBitstreamConsumeBits(stream, n);

    return r;
}

bool baseBitstreamPopBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out)
{
    *out = 0;
    for(u64 i = 0; i < n; i++)
    {
        u8 b;
        if (!baseBitstreamPopBit(stream, &b))
        {
            return false;
        }

        *out = (*out) | ((u64)b << i);
    }

    return true;
}
bool baseBitstreamPopU16LE(BaseBitstream *stream, u16 *out)
{
    u64 b1, b2;
    if(!baseBitstreamPopBitsAsU64(stream, 8, &b1) ||
       !baseBitstreamPopBitsAsU64(stream, 8, &b2))
    {
        return false;
    }

    *out = (u16)(b1 | (b2 << 8));

    return true;
}
bool baseBitstreamPopU16BE(BaseBitstream *stream, u16 *out)
{
    u64 b1, b2;
    if(!baseBitstreamPopBitsAsU64(stream, 8, &b1) ||
       !baseBitstreamPopBitsAsU64(stream, 8, &b2))
    {
        return false;
    }

    *out = (u16)(b2 | (b1 << 8));

    return true;
}

void baseBitstreamPopTillNextByte(BaseBitstream *stream)
{
    while(stream->bitIndex % 8 != 0)
    {
        baseBitstreamConsumeBits(stream, 1);
    }
}

void baseBitstreamConsumeBits(BaseBitstream *stream, u64 n)
{
    stream->bitIndex += n;
}
bool baseBitstreamPeekBit(BaseBitstream *stream, u8 *out)
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
bool baseBitstreamPeekBitsAsU8Impl(BaseBitstream *stream, u8 n, u8 *out)
{
    bool result = true;

    *out = 0;

    u64 i;
    for(i = 0; i < n; i++)
    {
        u8 b;
        if (!baseBitstreamPeekBit(stream, &b))
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
bool baseBitstreamPeekBitsAsU64Impl(BaseBitstream *stream, u64 n, u64 *out)
{
    bool result = true;

    *out = 0;

    u64 i;
    for(i = 0; i < n; i++)
    {
        u8 b;
        if (!baseBitstreamPeekBit(stream, &b))
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

bool baseBitstreamPeekBitsReversedAsU8Impl(BaseBitstream *stream, u8 n, u8 *out)
{
    u8 code = 0;
    if(baseBitstreamPeekBitsAsU8Impl(stream, n, &code))
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
bool baseBitstreamPeekBitsReversedAsU64Impl(BaseBitstream *stream, u64 n, u64 *out)
{
    u64 code = 0;
    if(baseBitstreamPeekBitsAsU64Impl(stream, n, &code))
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