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
    CMDLINE_ARG_STR8LIST,
    CMDLINE_ARG_REF,
}CmdlineArgKind;

typedef struct CmdlineArgValue
{
    CmdlineArgKind kind;
    union
    {
        i64 asI64;
        bool asBool;
        str8 asStr8;
        Str8List asStr8list;
        void *asRef;
    }_union;
}CmdlineArgValue;

typedef struct CmdlineArgDef
{
    CmdlineArgValue defValue;
    CmdlineArgValue value;
    str8 name;
    str8 help;
    CmdlineArgPresenceKind presence;
    bool passed;
}CmdlineArgDef;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(CmdlineArgDefArray, CmdlineArgDef)

#define BASE_CMDLINE_MAX_ARG_DEF_COUNT 255
#define cmdlineStruct(T, S) (cmdlineStructEx(S, g##T##MembDefsTable))

i64 *cmdlineI64(str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence);
void cmdlineI64Var(i64 *arg, str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence);
bool *cmdlineBool(str8 name, bool def, str8 help, CmdlineArgPresenceKind presence);
void cmdlineBoolVar(bool *arg, str8 name, bool def, str8 help, CmdlineArgPresenceKind presence);
str8 *cmdlineStr8(str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence);
void cmdlineStr8Var(str8 *arg, str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence);
Str8List *cmdlineStr8List(str8 name, Str8List def, str8 help, CmdlineArgPresenceKind presence);
void cmdlineStr8ListVar(Str8List *arg, str8 name, Str8List def, str8 help, CmdlineArgPresenceKind presence);
Str8List *cmdlineTrailing(str8 help);
bool cmdlineParse(Str8List cmdline);
bool cmdlineStructEx(void *s, MetagenStructMembArray membs);
void cmdlineUsage(void);
#endif