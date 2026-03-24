#include "base/baseUri.h"
#include "os/net/osNetHTTP.h"

BASE_CREATE_EFFICIENT_LL_DEFS(OSNetHttpHeaderList, OSNetHttpHeader);

bool OSNetHttpIsToken(u8 c)
{
    return (BASE_ISALPHA(c) || BASE_ISDIGIT(c) || 
            c == '!' || c == '#' || c == '%' ||
            c == '$' || c == '&' || c == '\'' ||
            c == '*' || c == '+' || c == '-' || 
            c == '.' || c == '^' || c == '_' ||
            c == '`' || c == '|' || c == '~');
}

str8 OSNetHttpGetNextTokenFromStr8(str8 str)
{
    str8 tok = str;
    tok.len = 0;

    if (!BASE_NULL_OR_EMPTY(str))
    {
        for(; tok.len < str.len && OSNetHttpIsToken(str.data[tok.len]); tok.len++);
    }

    return tok;
}

OSNetHttpHeader OSNetHttpHeaderFromStr8(Arena *arena, str8 str);
OSNetHttpHeaderList OSNetHttpHeaderListFromStr8(Arena *arena, str8 str);
OSNetHttpPacket OSNetHttpPacketFromStr8(Arena *arena, str8 str)
{
    OSNetHttpPacket request = {0};

    if (!BASE_NULL_OR_EMPTY(str))
    {
        str8 firstToken = OSNetHttpGetNextTokenFromStr8(str);
        if (!BASE_NULL_OR_EMPTY(firstToken))
        {
            str = Str8Skip(str, firstToken.len);

            bool isResponse = Str8StartsWith(firstToken, STR8_LIT("HTTP"), STR_MATCHFLAGS_CASE_INSENSITIVE);
            
            if (!isResponse)
            {
                request.method = Str8PushCopy(arena, firstToken);
                if (str.data[0] == ' ')
                {
                    str = Str8Skip(str, 1);

                    if (str.data[0] == '*')
                    {
                        request.resource = STR8_LIT("*");
                    }
                    else
                    {
                        str8 resource = {0};

                        for(; resource.len < str.len && str.data[resource.len] != ' '; resource.len++);

                        if (resource.len > 0)
                        {
                            resource.data = str.data;
                            request.resource = Str8PushCopy(arena, resource);
                        }
                    }

                    str = Str8Skip(str, request.resource.len);

                    if (str.data[0] == ' ')
                    {
                        str = Str8Skip(str, 1);
                        if(Str8Equals(OSNetHttpGetNextTokenFromStr8(str), STR8_LIT("HTTP"), STR_MATCHFLAGS_CASE_INSENSITIVE))
                        {
                            str = Str8Skip(str, 4);
                            if (str.data[0] == '/')
                            {
                                str = Str8Skip(str, 1);
                                request.httpVersion = Str8PushCopy(arena, OSNetHttpGetNextTokenFromStr8(str));

                                str = Str8Skip(str, request.httpVersion.len);
                            }
                        }
                    }
                }
            }
            else
            {
                if (str.data[0] == '/')
                {
                    str = Str8Skip(str, 1);
                    request.httpVersion = Str8PushCopy(arena, OSNetHttpGetNextTokenFromStr8(str));

                    str = Str8Skip(str, request.httpVersion.len);
                    
                    if (str.data[0] == ' ')
                    {
                        str = Str8Skip(str, 1);
                        request.status = (u16)U64FromStr8(Str8SubStr8(str, 0, 3));
                        str = Str8Skip(str, 3);

                        if (str.data[0] == ' ')
                        {
                            str = Str8Skip(str, 1);
                            if (str.data[0] != '\r')
                            {
                                u64 crlfIndex = Str8FindSubStr8(str, STR8_LIT("\r\n"), 0, STR_MATCHFLAGS_CASE_INSENSITIVE);
                                request.statusReason = Str8PushCopy(arena, Str8SubStr8(str, 0, crlfIndex));

                                str = Str8Skip(str, request.statusReason.len);
                            }
                        }
                    }
                }
            }

            while(Str8StartsWith(str, STR8_LIT("\r\n"), 0))
            {
                str = Str8Skip(str, STR8_LIT("\r\n").len);

                if(Str8StartsWith(str, STR8_LIT("\r\n"), 0))
                {
                    //msg body
                    str = Str8Skip(str, STR8_LIT("\r\n").len);
                    break;
                }

                str8 fieldName = OSNetHttpGetNextTokenFromStr8(str);
                if(!BASE_NULL_OR_EMPTY(fieldName))
                {
                    OSNetHttpHeader *header = arenaPushType(arena, OSNetHttpHeader);
                    header->name = Str8PushCopy(arena, fieldName);
                    
                    str = Str8Skip(str, fieldName.len);
                    if (str.data[0] == ':')
                    {
                        str = Str8Skip(str, 1);

                        str8 value = {0};
                        for(; value.len < str.len && str.data[value.len] != '\r'; value.len++);

                        if (value.len > 0)
                        {
                            value.data = str.data;
                            str = Str8Skip(str, value.len);

                            value = Str8Trim(value);
                            header->value = Str8PushCopy(arena, value);

                        }

                        OSNetHttpHeaderListPushNodeLast(&request.headers, header);
                    }
                }
            }

            if (str.len > 0)
            {
                // set remaining msg as the body
                str8 msg = Str8PushCopy(arena, str);
                request.messageBody.data = msg.data;
                request.messageBody.len = msg.len;
            }
        }
    }

    return request;
}