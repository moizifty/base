#ifndef BASE_URI_H
#define BASE_URI_H

#include "baseCore.h"
#include "baseCoreTypes.h"
#include "baseStrings.h"
#include "baseMemory.h"
#include "baseThreads.h"

typedef struct UriHierarchy
{
    str8 userinfo;
    str8 host;
    str8 port;
    str8 path;
}UriHierarchy;

typedef struct Uri
{
    str8 scheme;
    UriHierarchy hier;
    str8 query;
    str8 fragment;
}Uri;

Uri baseUriParseFromStr8(str8 str);
Uri baseUriParseFromStr8Copy(Arena *arena, str8 str);

#endif