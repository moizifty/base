#include "bssCore.h"

void BssTokChunkListPushLast(Arena *arena, BssTokChunkList *l, BssTok tok)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        BssTokChunkListNode *n = arenaPushType(arena, BssTokChunkListNode);
        n->cap = 50;
        n->chunk.data = arenaPushArray(arena, BssTok, n->cap);
        n->chunk.len = 0;

        BasePtrListNodePushLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = tok;
    l->last->chunk.len += 1;
    l->totalLen += 1;
}
BssTokArray BssTokChunkListFlattenToArray(Arena *arena, BssTokChunkList *l)
{
    BssTokArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = arenaPushArray(arena, BssTok, l->totalLen);
    flattened.len = l->totalLen;

    u64 i = 0;
    BASE_PTR_LIST_FOREACH(BssTokChunkListNode, chunk, l)
    {
        for(u64 j = 0; j < chunk->chunk.len; j++)
        {
            flattened.data[i++] = chunk->chunk.data[j];
        }
    }

    return flattened;
}

i64 bssGetEscapeCharValue(str8 escapeCharString)
{
    if(escapeCharString.data[0] != '~')
    {
        return -1;
    }

    if(escapeCharString.len == 2)
    {
        i8 ch = escapeCharString.data[1];

        switch(ch)
        {
            case '\'': return ch;
            case '\"': return ch;
            case '~': return ch;
            case '0': return '\0';
            case 'r': return '\r';
            case 'n': return '\n';
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 't': return '\t';
            case 'v': return '\v';
            default: return -1;
        }
    }
    else if(escapeCharString.len == 6) 
    {
        // length six for unicode chars when tht is done eg \uxxxx where xxxx is a hex number
    }

    return -1;
}

void bssPrintSourceRange(BssTokPos start, BssTokPos end, u64 contextLines)
{
    u8 *lexerBufStart = start.ownerLexer->buffer.data;
    u8 *lexerBufEnd = start.ownerLexer->buffer.data + start.ownerLexer->buffer.len;

    u8 *rangeStart = start.range.data;
    u8 *rangeEnd = end.range.data + end.range.len;

    u8 *printStart = rangeStart;
    u8 *printEnd = rangeEnd;

    // u wana compare against contextline + 1 since you also want to print the COMPLETE LINE
    // u dont just wana quit at the newline otherwise ud be one context line short
    for(u64 newlinesSeen = 0; printStart >= lexerBufStart && newlinesSeen != (contextLines + 1); )
    {
        if (*printStart == '\n')
        {
            newlinesSeen++;
            if (newlinesSeen == (contextLines + 1)) break;
        }
        
        printStart--;
    }

    /// do the same for end
    for(u64 newlinesSeen = 0; printEnd < lexerBufEnd && newlinesSeen != (contextLines + 1); )
    {
        if (*printEnd == '\n')
        {
            newlinesSeen++;
            if (newlinesSeen == (contextLines + 1)) break;
        }
        
        printEnd++;
    }

    while (printStart != printEnd)
    {
        basePrintf("%c", *printStart);
        printStart++;
    }

    basePrintf("\n");
}