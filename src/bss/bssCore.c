#include "bssCore.h"
#include "bssScope.h"

readonly BssValue gBssValueEmpty = {0};
readonly BssValue gBssValueVoid = {.kind = BSS_VALUE_VOID};
readonly BssBuiltinFunc gBssBuiltinFuncEmpty = {0};

BASE_CREATE_LL_DEFS(BssTokList, BssTok)
BASE_CREATE_EFFICIENT_LL_DEFS(BssBuiltinFuncList, BssBuiltinFunc)
BASE_CREATE_EFFICIENT_LL_DEFS(BssTokFmtStrPosList, BssTokFmtStrPos)
BASE_CREATE_EFFICIENT_LL_DEFS(BssValueList, BssValue)

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

str8 bssGetStr8RepFromTokLexeme(Arena *arena, BssTok tok)
{
    str8 ret = {0};
    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        // '""
        tok.lexeme.data++;

        // the speech marks
        tok.lexeme.len -= 2;

        str8 t = {0}; 
        t.data = arenaPushArray(temp.arena, u8, tok.lexeme.len);
        t.len = 0;

        for(u64 i = 0; i < tok.lexeme.len; i++)
        {
            u8 ch = tok.lexeme.data[i];
            u8 toWrite = ch;

            if (ch == '~')
            {
                str8 charLitStr = {0};
                charLitStr.data = tok.lexeme.data + i;
                charLitStr.len = 2;

                i64 ev = bssGetEscapeCharValue(charLitStr);
                toWrite = (u8) ev;

                i++;
            }

            t.data[t.len++] = toWrite;
        }

        ret = Str8PushCopy(arena, t);

    }
    arenaTempEnd(temp);

    return ret;
}

void bssBuiltinFunctionPushEntry(BssInterp *interp, str8 name, int numParams, BssFunc fn)
{
    BssBuiltinFunc *builtin = arenaPushType(interp->arena, BssBuiltinFunc);
    builtin->name = name;
    builtin->func = fn;

    BssBuiltinFuncListPushNodeLast(&interp->builtins, builtin);

    BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
    bssScopePushEntry(interp->rootScope, name, &entry);

    entry->name = name;
    entry->value = arenaPushType(interp->arena, BssValue);
    entry->value->kind = BSS_VALUE_FUNCTION;
    entry->value->fn.isBuiltin = true;
    entry->value->fn.builtin.fn = builtin;
    entry->value->fn.builtin.numParams = numParams;
}
BssBuiltinFunc *bssBuiltinFunctionFindEntry(BssInterp *interp, str8 name)
{
    BssBuiltinFunc *found = BSS_BUILTIN_FUNC_ZERO;
    BASE_LIST_FOREACH(BssBuiltinFunc, func, interp->builtins)
    {
        if (Str8Equals(func->name, name, 0))
        {
            return func;
        }
    }


    return found;
}

void bssPrintSourceRange(BssTokPos start, BssTokPos end, u64 contextLines)
{
    u8 *lexerBufStart = start.ownerLexer->buffer.data;
    u8 *lexerBufEnd = start.ownerLexer->buffer.data + start.ownerLexer->buffer.len;

    u8 *rangeStart = start.range.data;
    u8 *rangeEnd = end.range.data + end.range.len;

    u8 *printStart = rangeStart;
    u8 *printEnd = rangeEnd;

    u64 numLinesReversed = 0;
    u64 numLinesForwarded = 0;
    // u wana compare against contextline + 1 since you also want to print the COMPLETE LINE
    // u dont just wana quit at the newline otherwise ud be one context line short
    for(; printStart > lexerBufStart && numLinesReversed != (contextLines + 1); )
    {
        if (*printStart == '\n')
        {
            numLinesReversed++;
            if (numLinesReversed == (contextLines + 1)) break;
        }
        
        printStart--;
    }

    /// do the same for end
    for(; printEnd < lexerBufEnd && numLinesForwarded != (contextLines + 1); )
    {
        if (*printEnd == '\n')
        {
            numLinesForwarded++;
            if (numLinesForwarded == (contextLines + 1)) break;
        }
        
        printEnd++;
    }

    bool emitLineInfo = true;
    u64 currLineNumber = start.line - numLinesReversed;
    while (printStart != printEnd)
    {
        bool isNewline = *printStart == '\n';

        if (*printStart == '\r' || *printStart == '\f' || *printStart == '\v')
        {
            printStart++;
            continue;
        }

        if (emitLineInfo)
        {
            basePrintf("\n%7lld | ", currLineNumber);
            emitLineInfo = false;
        }

        if (isNewline)
        {
            emitLineInfo = true;
            currLineNumber++;
        }
        else
        {
            if (printStart >= rangeStart && printStart < rangeEnd) basePrintf("{r}%c", *printStart);
            else basePrintf("%c", *printStart);
        }

        printStart++;
    }

    basePrintf("\n");
}

BssValue *bssAllocValue(Arena *arena, BssValueKind kind)
{
    BssValue *value = arenaPushType(arena, BssValue);
    value->kind = kind;

    return value;
}
BssValue *bssAllocValueCopy(Arena *arena, BssValue *other)
{
    BssValue *value = arenaPushType(arena, BssValue);
    value->kind = other->kind;
    *value = *other;

    // handle all allocated types
    if (value->kind == BSS_VALUE_STRING)
    {
        value->str = Str8PushCopy(arena, other->str);
    }
    else if (value->kind == BSS_VALUE_OBJECT)
    {
        value->obj = bssAllocScope(arena, null, false);

        BASE_LIST_FOREACH(BssSymTableSlotEntry, entry, other->obj->symTable.entries)
        {
            BssSymTableSlotEntry *copyEntry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
            bssScopePushEntry(value->obj, entry->name, &copyEntry);
            copyEntry->value = bssAllocValueCopy(arena, entry->value);
        }
    }
    else if(value->kind == BSS_VALUE_ARRAY)
    {
        value->array = (BssValueList){0};
        BASE_LIST_FOREACH(BssValue, v, other->array)
        {
            BssValue *copiedSubItem = bssAllocValueCopy(arena, v);
            BssValueListPushNodeLast(&value->array, copiedSubItem);
        }
    }

    return value;
}
BssValue *bssAllocValueInt(Arena *arena, i64 val)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_INT);
    value->num = val;

    return value;
}
BssValue *bssAllocValueStr8(Arena *arena, str8 val)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_STRING);
    value->str = val;

    return value;
}
BssValue *bssAllocValueBool(Arena *arena, bool val)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_BOOL);
    value->num = val;

    return value;
}
BssValue *bssAllocValueFn(Arena *arena, struct BssAstFunc *ast)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_FUNCTION);
    value->fn.defined.ast = ast;

    return value;
}
BssValue *bssAllocValueArray(Arena *arena, BssValueList values)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_ARRAY);
    value->array = values;

    return value;
}
BssValue *bssAllocValueObj(Arena *arena, BssScope *scope)
{
    BssValue *value = bssAllocValue(arena, BSS_VALUE_OBJECT);
    value->obj = scope;

    return value;
}

str8 Str8FromBssValue(Arena *arena, BssValue *value)
{
    switch(value->kind)
    {
        case BSS_VALUE_VOID: return STR8_LIT("void");
        case BSS_VALUE_INT: return Str8PushFmt(arena, "%lld", value->num);
        case BSS_VALUE_BOOL: return value->num ? STR8_LIT("true") : STR8_LIT("false");
        case BSS_VALUE_STRING: return value->str;
        case BSS_VALUE_OBJECT:
        {
            str8 val = STR8_EMPTY;
            ArenaTemp temp = baseTempBegin(&arena, 1);
            {
                Str8List list = {0};
                Str8ListPushLastFmt(temp.arena, &list, "{");
                BASE_LIST_FOREACH(BssSymTableSlotEntry, entry, value->obj->symTable.entries)
                {
                    Str8ListPushLastFmt(temp.arena, &list, "%S = %S", entry->name, Str8FromBssValue(temp.arena, entry->value));
                    if (entry != value->obj->symTable.entries.last)
                    {
                        Str8ListPushLastFmt(temp.arena, &list, ", ");
                    }
                }
                Str8ListPushLastFmt(temp.arena, &list, "}");

                val = Str8ListJoin(arena, &list, null);
            }

            baseTempEnd(temp);

            return val;
        }break;
        case BSS_VALUE_ARRAY:
        {
            str8 val = STR8_EMPTY;
            ArenaTemp temp = baseTempBegin(&arena, 1);
            {
                Str8List list = {0};
                Str8ListPushLastFmt(temp.arena, &list, "{");
                BASE_LIST_FOREACH(BssValue, v, value->array)
                {
                    Str8ListPushLast(temp.arena, &list, Str8FromBssValue(temp.arena, v));
                    if (v != value->array.last)
                    {
                        Str8ListPushLastFmt(temp.arena, &list, ", ");
                    }
                }
                Str8ListPushLastFmt(temp.arena, &list, "}");

                val = Str8ListJoin(arena, &list, null);
            }

            baseTempEnd(temp);

            return val;
        }break;

        default:
        {
            baseEPrintf("{r}Failed to convert bssvalue to str8\n");
            return STR8_EMPTY;
        }
    }
}