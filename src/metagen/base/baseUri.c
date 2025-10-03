#include "base\baseUri.h"

#define BASE_URI_MATCH(STR, EXPECTED, INDEX) (((STR).len > (INDEX)) ? ((STR).data[(INDEX)] == (EXPECTED)) : false)

bool baseUriCharacterIsUnreserved(u8 ch)
{
    return BASE_ISALPHA(ch) || BASE_ISDIGIT(ch) || (ch == '-') || (ch == '.') || (ch == '_') || (ch == '~');
}

bool baseUriCharacterIsGenDelims(u8 ch)
{
    return (ch == ':') || (ch == '/') || (ch == '?') || (ch == '#') || (ch == '[') || (ch == ']') || (ch == '@');
}

bool baseUriCharacterIsSubDelims(u8 ch)
{
    return (ch == '!') || (ch == '$') || (ch == '&') || (ch == '\'') || (ch == '(') || (ch == ')') || (ch == '*') ||
           (ch == '+') || (ch == ',') || (ch == ';') || (ch == '=');
}

bool baseUriIsPCTEncoded(str8 str, u64 index)
{
    if (index <= str.len - 3)
    {
        return BASE_URI_MATCH(str, '%', index) && BASE_ISHEXDIGIT(str.data[index + 1]) && BASE_ISHEXDIGIT(str.data[index + 1]);
    }

    return false;
}

// https://datatracker.ietf.org/doc/html/rfc3986
Uri baseUriParseFromStr8(str8 str)
{
    Uri ret = {0};
    if (!BASE_ANY(str))
    {
        return ret;
    }

    // URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

    // parse the scheme:
    // scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    if (BASE_ISALPHA(str.data[0]))
    {
        u64 i = 0;
        while(i < str.len && 
              str.data[i] != ':' && 
              ((BASE_ISALPHA(str.data[i]) || 
                BASE_ISDIGIT(str.data[i]) || 
                (str.data[i] == '+') || 
                (str.data[i] == '-') || 
                (str.data[i] == '.'))))
        {
            i++;
        }

        if (BASE_URI_MATCH(str, ':', i))
        {
            ret.scheme = baseStr8(str.data, i);
            i += 1;

            /*
             *  hier-part   = "//" authority path-abempty
                  / path-absolute
                  / path-rootless
                  / path-empty
                authority     = [ userinfo "@" ] host [ ":" port ]
                userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
                host          = IP-literal / IPv4address / reg-name
                port          = *DIGIT
            */

            if (BASE_URI_MATCH(str, '/', i) &&
                BASE_URI_MATCH(str, '/', i + 1))
            {
                i+=2; // '//'
                
                {
                    // parse authority then path
                    u64 authStartIndex = i;
                    u64 authEndIndex = i;
                    while (authEndIndex < str.len && 
                        (str.data[authEndIndex] != '/') &&
                        (str.data[authEndIndex] != '?') &&
                        (str.data[authEndIndex] != '#'))
                    {
                        authEndIndex += 1;
                    }

                    str8 auth = baseStr8(str.data + authStartIndex, authEndIndex - authStartIndex);
                    
                    if(Str8Contains(auth, '@'))
                    {
                        u64 userInfoStartIndex = i;

                        // userinfo
                        while (i < str.len &&
                            !BASE_URI_MATCH(str, '@', i) &&
                            (baseUriCharacterIsUnreserved(str.data[i]) ||
                                baseUriCharacterIsSubDelims(str.data[i]) ||
                                baseUriIsPCTEncoded(str, i) ||
                                str.data[i] == ':'))
                        {
                            i += (str.data[i] == '%') ? 3 : 1;
                        }

                        ret.hier.userinfo = baseStr8(auth.data, i - userInfoStartIndex);

                        i += 1; // @
                    }

                    str8 hostAndMaybePort = 
                    {
                        .data = str.data + i, 
                        .len = (auth.len - ret.hier.userinfo.len) - (BASE_ANY(ret.hier.userinfo) ? 1 : 0),
                    };

                    if (Str8Contains(hostAndMaybePort, ':'))
                    {
                        u64 colonIndex = Str8FindSubStr8(hostAndMaybePort, STR8_LIT(":"), 0, STR_MATCHFLAGS_FIND_LAST);
                        ret.hier.port = baseStr8(hostAndMaybePort.data + colonIndex + 1, hostAndMaybePort.len - colonIndex - 1);

                        i += 1;
                    }

                    ret.hier.host = (str8)
                    {
                        .data = hostAndMaybePort.data, 
                        .len = hostAndMaybePort.len - ret.hier.port.len - (BASE_ANY(ret.hier.port) ? 1 : 0), // -1 for ':'
                    };

                    i += ret.hier.host.len + ret.hier.port.len;
                }

                {
                    u64 pathStart = i;

                    while(i < str.len &&
                          BASE_URI_MATCH(str, '/', i))
                    {
                        i += 1;

                        while (i < str.len &&
                               (baseUriCharacterIsUnreserved(str.data[i]) ||
                                baseUriCharacterIsSubDelims(str.data[i]) ||
                                baseUriIsPCTEncoded(str, i) ||
                                BASE_URI_MATCH(str, ':', i) ||
                                BASE_URI_MATCH(str, '@', i)))
                        {
                            i += 1;
                        }
                    }

                    ret.hier.path = baseStr8(str.data + pathStart, i - pathStart);
                }
            }

            // do query
            {
                str8 maybeQueryAndMaybeFragment = baseStr8(str.data + i, str.len - i);
                if (BASE_ANY(maybeQueryAndMaybeFragment))
                {
                    u64 hashTagIndex = Str8FindSubStr8(maybeQueryAndMaybeFragment, STR8_LIT("#"), 0, STR_MATCHFLAGS_FIND_LAST);
                    if (hashTagIndex != maybeQueryAndMaybeFragment.len)
                    {
                        ret.fragment = baseStr8(maybeQueryAndMaybeFragment.data + hashTagIndex + 1, maybeQueryAndMaybeFragment.len - hashTagIndex - 1);
                    }

                    ret.query = baseStr8(maybeQueryAndMaybeFragment.data + 1, maybeQueryAndMaybeFragment.len - ret.fragment.len - 2);
                }
            }
        }
    }

    return ret;
}

Uri baseUriParseFromStr8Copy(Arena *arena, str8 str)
{
    Uri uri = baseUriParseFromStr8(str);

    uri.scheme = Str8PushCopy(arena, uri.scheme);
    uri.hier.userinfo = Str8PushCopy(arena, uri.hier.userinfo);
    uri.hier.host = Str8PushCopy(arena, uri.hier.host);
    uri.hier.port = Str8PushCopy(arena, uri.hier.port);
    uri.hier.path = Str8PushCopy(arena, uri.hier.path);
    uri.query = Str8PushCopy(arena, uri.query);
    uri.fragment = Str8PushCopy(arena, uri.fragment);

    return uri;
}