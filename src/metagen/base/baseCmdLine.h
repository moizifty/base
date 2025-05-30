#ifndef BASE_CMD_LINE_H
#define BASE_CMD_LINE_H

#include "baseCore.h"
#include "baseMemory.h"
#include "baseStrings.h"
#include "baseHash.h"

typedef struct CmdLineOptNode CmdLineOptNode;
struct CmdLineOptNode
{
    CmdLineOptNode *next;
    CmdLineOptNode *prev;
    str8 option;
    Str8List values;
};

typedef struct CmdLineOptSlot
{
    CmdLineOptNode *first;
    CmdLineOptNode *last;
}CmdLineOptSlot;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(CmdLineOptSlotArray, CmdLineOptSlot);

typedef struct CmdLineHashMap
{
    CmdLineOptSlotArray slots;
    Str8List originalInputs;
}CmdLineHashMap;

u64 cmdlineGetHashOfOption(str8 option);
CmdLineHashMap cmdlineParseCmdLineFromStringList(BaseArena *arena, Str8List list);
str8 cmdlineGetStr8(CmdLineHashMap *map, str8 option);
bool cmdlineGetFlag(CmdLineHashMap *map, str8 option);
i64 cmdlineGetI64(CmdLineHashMap *map, str8 option);

#endif