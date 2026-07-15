#include "jsonParser.h"

JSONValue jsonParserTryParseStringLiteral(str8 jsonStr)
{
    JSONValue literal = {0};

    if (jsonStr.data[0] == '\"')
    {
        literal.strRep.data = jsonStr.data;
        literal.strRep.len++;

        jsonStr = Str8Skip(jsonStr, 1);

        while(jsonStr.len)
        {
            if (jsonStr.data[0] == '\\')
            {
                if (jsonStr.len && (jsonStr.data[0] == '\"'))
                {
                    jsonStr = Str8Skip(jsonStr, 2);
                    continue;
                }
            }
            else if (jsonStr.data[0] == '\"')
            {
                literal.strRep.len++;
                break;
            }
            literal.strRep.len++;
            jsonStr = Str8Skip(jsonStr, 1);
        }

        literal.kind = JSON_VALUE_STRING;
        literal.asStr8 = Str8SubStr8(literal.strRep, 1, literal.strRep.len - 1);
    }

    return literal;
}
JSONValue jsonParserTryParseLiteral(str8 jsonStr)
{
    JSONValue literal = {0};
    switch (jsonStr.data[0])
    {
        case 't':
        case 'f':
        case 'n':
        {
            literal.strRep.data = jsonStr.data;
            while (jsonStr.len && BASE_ISALPHA(*jsonStr.data))
            {
                literal.strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            if (Str8Equals(literal.strRep, STR8("false"), 0))
            {
                literal.kind = JSON_VALUE_BOOL;
                literal.asBool = false;
                return literal;
            }
            else if(Str8Equals(literal.strRep, STR8("true"), 0))
            {
                literal.kind = JSON_VALUE_BOOL;
                literal.asBool = true;
                return literal;
            }
            else if(Str8Equals(literal.strRep, STR8("null"), 0))
            {
                literal.kind = JSON_VALUE_NULL;
                return literal;
            }
            else
            {
                return literal;
            }
        }break;

        case '\"':
        {
            return jsonParserTryParseStringLiteral(jsonStr);
        }break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        {
            literal.strRep.data = jsonStr.data;

            if(jsonStr.data[0] == '-')
            {
                literal.strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            while(jsonStr.len && BASE_ISDIGIT(*jsonStr.data))
            {
                literal.strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            if (jsonStr.len && jsonStr.data[0] == '.')
            {
                literal.strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);

                while(jsonStr.len && BASE_ISDIGIT(*jsonStr.data))
                {
                    literal.strRep.len++;
                    jsonStr = Str8Skip(jsonStr, 1);
                }
            }


            // should really make f64tryfromstr8 allow taking a null;
            literal.kind = JSON_VALUE_NUMBER;
            F64TryFromStr8(literal.strRep, &literal.asNumber);
        }break;
    }

    return literal;
}
str8 jsonParserSkipWhitespace(str8 jsonStr)
{
    while (jsonStr.len && isspace(*jsonStr.data))
    {
        jsonStr = Str8Skip(jsonStr, 1);
    }

    return jsonStr;
}
JSONValue JSONValueFromStr8(Arena *arena, str8 str)
{
    JSONValue ret = {0};

    if (BASE_NULL_OR_EMPTY(str))
    {
        return ret;
    }

    str = jsonParserSkipWhitespace(str);

    switch (str.data[0])
    {
        case '{':
        {
            ret.kind = JSON_VALUE_OBJ;
            ret.strRep.data = str.data;
            str = Str8Skip(str, 1);

            while (str.len && str.data[0] != '}')
            {
                str = jsonParserSkipWhitespace(str);

                // maybe do this in temp arena first then copy after all valid obj membs have been parsed
                // because right now if the code fails, it still allocates, which isnt really ideal
                JSONObjMemb *memb = arenaPushType(arena, JSONObjMemb);
                JSONValue membNameValue = jsonParserTryParseStringLiteral(str);

                if (membNameValue.kind != JSON_VALUE_INVALID)
                {
                    memb->name = Str8PushFmt(arena, "%S", membNameValue.asStr8);

                    str = jsonParserSkipWhitespace(str);

                    if (str.data[0] == ':')
                    {
                        str = Str8Skip(str, 1);
                        memb->value = JSONValueFromStr8(arena, str);

                        str = jsonParserSkipWhitespace(str);

                        if (str.data[0] != '}')
                        {
                            if (str.data[0] != ',')
                            {
                                // error
                                break;
                            }
                            else
                            {
                                str = Str8Skip(str, 1);
                            }
                        }

                        JSONObjMembListPushNodeLast(&ret.asObj, memb);
                    }
                    else
                    {
                        // error
                        break;
                    }
                }
                else
                {
                    // error
                    break;
                }
            }
        }break;

        case '[':
        {
            ret.kind = JSON_VALUE_ARRAY;
            ret.strRep.data = str.data;
            str = Str8Skip(str, 1);

            while (str.len && str.data[0] != ']')
            {
                JSONValue value = JSONValueFromStr8(arena, str);
                if (value.kind != JSON_VALUE_INVALID)
                {
                    if (str.data[0] != ']')
                    {
                        if (str.data[0] != ',')
                        {
                            // error
                            break;
                        }
                        else
                        {
                            str = Str8Skip(str, 1);
                        }
                    }

                    JSONValueListPushLast(arena, &ret.asArray, value);
                }
                else
                {
                    // error
                    break;
                }

            }
        }break;

        case 't':
        case 'f':
        case 'n':
        case '\"':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            ret = jsonParserTryParseLiteral(str);

            if (ret.kind == JSON_VALUE_INVALID)
            {
                // error
            }
        }break;

        default:
        {
            // error
        }break;
    }

    return ret;
}