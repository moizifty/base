#ifndef BASE_CMD_LINE_H
#define BASE_CMD_LINE_H

#include "baseCore.h"
#include "baseMemory.h"
#include "baseStrings.h"

typedef enum CmdlineArgPresenceKind
{
    CMDLINE_ARG_PRESENCE_OPTIONAL,
    CMDLINE_ARG_PRESENCE_REQUIRED,
}CmdlineArgPresenceKind;

typedef enum CmdlineArgKind
{
    CMDLINE_ARG_NULL,
    CMDLINE_ARG_I64,
    CMDLINE_ARG_BOOL,
    CMDLINE_ARG_STR8,
}CmdlineArgKind;

typedef struct CmdlineArgDef
{
    CmdlineArgKind kind;
    union
    {
        i64 asI64;
        bool asBool;
        str8 asStr8;
    };

    str8 name;
    str8 help;
    CmdlineArgPresenceKind presence;
    bool passed;
}CmdlineArgDef;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(CmdlineArgDefArray, CmdlineArgDef)

#define BASE_CMDLINE_MAX_ARG_DEF_COUNT 255

i64 *cmdlineI64(str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence);
bool *cmdlineBool(str8 name, bool def, str8 help, CmdlineArgPresenceKind presence);
str8 *cmdlineStr8(str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence);
Str8List *cmdlineTrailing(void);
bool cmdlineParse(Str8List cmdline);
#endif