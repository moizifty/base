#ifndef BASE_URI_H
#define BASE_URI_H

#include "baseCore.h"
#include "baseCoreTypes.h"
#include "baseStrings.h"
#include "baseMemory.h"
#include "baseThreads.h"

typedef struct BaseUriHierarchy
{
    str8 userinfo;
    str8 host;
    str8 port;
    str8 path;
}BaseUriHierarchy;

typedef struct BaseUri
{
    str8 scheme;
    BaseUriHierarchy hier;
    str8 query;
    str8 fragment;
}BaseUri;

BaseUri baseUriParseFromStr8(str8 str);
BaseUri baseUriParseFromStr8Copy(BaseArena *arena, str8 str);

#endif