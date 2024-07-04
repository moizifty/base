#include "base\baseThreads.h"
#include "bssCore.h"

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

str8 bssGetStr8RepFromTokLexeme(BaseArena *arena, BssTok tok)
{
    str8 ret = {0};
    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        // '""
        tok.lexeme.data++;

        // the speech marks
        tok.lexeme.len -= 2;

        str8 t = {0}; 
        t.data = baseArenaPushArray(temp.arena, u8, tok.lexeme.len);
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

        ret = baseStringsPushStr8Copy(arena, t);

    }
    baseArenaTempEnd(temp);

    return ret;
}