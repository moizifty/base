#include "jsonParser.h"

bool jsonParserTryParseStringLiteral(str8 jsonStr, JSONValue *literal)
{
    if (jsonStr.data[0] == '\"')
    {
        literal->strRep.data = jsonStr.data;
        literal->strRep.len++;

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
                literal->strRep.len++;
                break;
            }
            literal->strRep.len++;
            jsonStr = Str8Skip(jsonStr, 1);
        }

        literal->kind = JSON_VALUE_STRING;
        literal->asStr8 = Str8SubStr8(literal->strRep, 1, literal->strRep.len - 1);
        return true;
    }

    return false;
}
bool jsonParserTryParseLiteral(str8 jsonStr, JSONValue *literal)
{
    *literal = (JSONValue){0};
    switch (jsonStr.data[0])
    {
        case 't':
        case 'f':
        case 'n':
        {
            literal->strRep.data = jsonStr.data;
            while (jsonStr.len && BASE_ISALPHA(*jsonStr.data))
            {
                literal->strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            if (Str8Equals(literal->strRep, STR8("false"), 0))
            {
                literal->kind = JSON_VALUE_BOOL;
                literal->asBool = false;
                return true;
            }
            else if(Str8Equals(literal->strRep, STR8("true"), 0))
            {
                literal->kind = JSON_VALUE_BOOL;
                literal->asBool = true;
                return true;
            }
            else if(Str8Equals(literal->strRep, STR8("null"), 0))
            {
                literal->kind = JSON_VALUE_NULL;
                return true;
            }
            else
            {
                return false;
            }
        }break;

        case '\"':
        {
            return jsonParserTryParseStringLiteral(jsonStr, literal);
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
            literal->strRep.data = jsonStr.data;

            if(jsonStr.data[0] == '-')
            {
                literal->strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            while(jsonStr.len && BASE_ISDIGIT(*jsonStr.data))
            {
                literal->strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);
            }

            if (jsonStr.len && jsonStr.data[0] == '.')
            {
                literal->strRep.len++;
                jsonStr = Str8Skip(jsonStr, 1);

                while(jsonStr.len && BASE_ISDIGIT(*jsonStr.data))
                {
                    literal->strRep.len++;
                    jsonStr = Str8Skip(jsonStr, 1);
                }
            }


            // should really make f64tryfromstr8 allow taking a null;
            literal->kind = JSON_VALUE_NUMBER;
            return F64TryFromStr8(literal->strRep, &literal->asNumber);
        }break;
    }

    return false;
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
                JSONValue membNameValue = {0};
                if(jsonParserTryParseStringLiteral(str, &membNameValue))
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
                    JSONValueListPushLast(arena, &ret.asArray, value);

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
            if (!jsonParserTryParseLiteral(str, &ret))
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