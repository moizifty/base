#include "base\baseUri.h"
#include "os\net\osNetHTTP.h"

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

OSNetHttpHeader OSNetHttpHeaderFromStr8(BaseArena *arena, str8 str);
OSNetHttpHeaderList OSNetHttpHeaderListFromStr8(BaseArena *arena, str8 str);
OSNetHttpPacket OSNetHttpPacketFromStr8(BaseArena *arena, str8 str)
{
    OSNetHttpPacket request = {0};

    if (!BASE_NULL_OR_EMPTY(str))
    {
        str8 firstToken = OSNetHttpGetNextTokenFromStr8(str);
        if (!BASE_NULL_OR_EMPTY(firstToken))
        {
            str = baseStringsStrSkip(str, firstToken.len);

            bool isResponse = baseStringsStrStartsWith(firstToken, STR8_LIT("HTTP"), STR_MATCHFLAGS_CASE_INSENSITIVE);
            
            if (!isResponse)
            {
                request.method = baseStringsPushStr8Copy(arena, firstToken);
                if (str.data[0] == ' ')
                {
                    str = baseStringsStrSkip(str, 1);

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
                            request.resource = baseStringsPushStr8Copy(arena, resource);
                        }
                    }

                    str = baseStringsStrSkip(str, request.resource.len);

                    if (str.data[0] == ' ')
                    {
                        str = baseStringsStrSkip(str, 1);
                        if(baseStringsStrEquals(OSNetHttpGetNextTokenFromStr8(str), STR8_LIT("HTTP"), STR_MATCHFLAGS_CASE_INSENSITIVE))
                        {
                            str = baseStringsStrSkip(str, 4);
                            if (str.data[0] == '/')
                            {
                                str = baseStringsStrSkip(str, 1);
                                request.httpVersion = baseStringsPushStr8Copy(arena, OSNetHttpGetNextTokenFromStr8(str));

                                str = baseStringsStrSkip(str, request.httpVersion.len);
                            }
                        }
                    }
                }
            }
            else
            {
                if (str.data[0] == '/')
                {
                    str = baseStringsStrSkip(str, 1);
                    request.httpVersion = baseStringsPushStr8Copy(arena, OSNetHttpGetNextTokenFromStr8(str));

                    str = baseStringsStrSkip(str, request.httpVersion.len);
                    
                    if (str.data[0] == ' ')
                    {
                        str = baseStringsStrSkip(str, 1);
                        request.status = (u16)baseU64FromStr8(baseStringsStrSubStr8(str, 0, 3));
                        str = baseStringsStrSkip(str, 3);

                        if (str.data[0] == ' ')
                        {
                            str = baseStringsStrSkip(str, 1);
                            if (str.data[0] != '\r')
                            {
                                u64 crlfIndex = baseStringsStrFindSubStr8(str, STR8_LIT("\r\n"), 0, STR_MATCHFLAGS_CASE_INSENSITIVE);
                                request.statusReason = baseStringsPushStr8Copy(arena, baseStringsStrSubStr8(str, 0, crlfIndex));

                                str = baseStringsStrSkip(str, request.statusReason.len);
                            }
                        }
                    }
                }
            }

            while(baseStringsStrStartsWith(str, STR8_LIT("\r\n"), 0))
            {
                str = baseStringsStrSkip(str, STR8_LIT("\r\n").len);

                if(baseStringsStrStartsWith(str, STR8_LIT("\r\n"), 0))
                {
                    //msg body
                    str = baseStringsStrSkip(str, STR8_LIT("\r\n").len);
                    break;
                }

                str8 fieldName = OSNetHttpGetNextTokenFromStr8(str);
                if(!BASE_NULL_OR_EMPTY(fieldName))
                {
                    OSNetHttpHeader *header = baseArenaPushType(arena, OSNetHttpHeader);
                    header->name = baseStringsPushStr8Copy(arena, fieldName);
                    
                    str = baseStringsStrSkip(str, fieldName.len);
                    if (str.data[0] == ':')
                    {
                        str = baseStringsStrSkip(str, 1);

                        str8 value = {0};
                        for(; value.len < str.len && str.data[value.len] != '\r'; value.len++);

                        if (value.len > 0)
                        {
                            value.data = str.data;
                            str = baseStringsStrSkip(str, value.len);

                            value = baseStringsStrTrim(value);
                            header->value = baseStringsPushStr8Copy(arena, value);

                        }

                        OSNetHttpHeaderListPushNodeLast(&request.headers, header);
                    }
                }
            }

            if (str.len > 0)
            {
                // set remaining msg as the body
                str8 msg = baseStringsPushStr8Copy(arena, str);
                request.messageBody.data = msg.data;
                request.messageBody.len = msg.len;
            }
        }
    }

    return request;
}