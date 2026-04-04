#include "baseCmdLine.h"
#include "baseThreads.h"
#include "baseMemory.h"
#include "baseStrings.h"
#include "os/core/osCore.h"

CmdlineArgDefArray gBaseCmdlineArgDefs = 
{
    .data = (CmdlineArgDef[BASE_CMDLINE_MAX_ARG_DEF_COUNT]){0},
    .len = 0,
};

Str8List gBaseCmdlineTrailingArgs = {0};
str8 gBaseCmdlineTrailingHelp = {0};

i64 *cmdlineI64(str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asI64 = def;
    nextDef->defValue.kind = CMDLINE_ARG_I64;
    nextDef->value._union.asI64 = def;
    nextDef->value.kind = CMDLINE_ARG_I64;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->value._union.asI64;
}
void cmdlineI64Var(i64 *arg, str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asI64 = def;
    nextDef->defValue.kind = CMDLINE_ARG_I64;
    nextDef->value._union.asRef = arg;
    nextDef->value.kind = CMDLINE_ARG_REF;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    *arg = def;
}
bool *cmdlineBool(str8 name, bool def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asBool = def;
    nextDef->defValue.kind = CMDLINE_ARG_BOOL;
    nextDef->value._union.asBool = def;
    nextDef->value.kind = CMDLINE_ARG_BOOL;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->value._union.asBool;
}
void cmdlineBoolVar(bool *arg, str8 name, bool def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asBool = def;
    nextDef->defValue.kind = CMDLINE_ARG_BOOL;
    nextDef->value._union.asRef = arg;
    nextDef->value.kind = CMDLINE_ARG_REF;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;
}
str8 *cmdlineStr8(str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asStr8 = def;
    nextDef->defValue.kind = CMDLINE_ARG_STR8;
    nextDef->value._union.asStr8 = def;
    nextDef->value.kind = CMDLINE_ARG_STR8;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->value._union.asStr8;
}
void cmdlineStr8Var(str8 *arg, str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asStr8 = def;
    nextDef->defValue.kind = CMDLINE_ARG_STR8;
    nextDef->value._union.asRef = arg;
    nextDef->value.kind = CMDLINE_ARG_REF;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;
}
Str8List *cmdlineStr8List(str8 name, Str8List def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asStr8list = def;
    nextDef->defValue.kind = CMDLINE_ARG_I64;
    nextDef->value._union.asStr8list = def;
    nextDef->value.kind = CMDLINE_ARG_I64;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->value._union.asStr8list;
}
void cmdlineStr8ListVar(Str8List *arg, str8 name, Str8List def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->defValue._union.asStr8list = def;
    nextDef->defValue.kind = CMDLINE_ARG_I64;
    nextDef->value._union.asRef = arg;
    nextDef->value.kind = CMDLINE_ARG_REF;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;
}
bool cmdlineParse(Str8List cmdline)
{
    if (!BASE_ANY(gBaseCmdlineArgDefs) && BASE_ANY(cmdline) && BASE_NULL_OR_EMPTY(gBaseCmdlineTrailingHelp))
    {
        baseEPrintf("{r}No arguments are required by the program but some are passed.\n");
        return false;
    }
    
    bool result = true;
    BASE_LIST_FOREACH(Str8ListNode, node, cmdline)
    {
        bool found = false;

        str8 arg = node->val;
        bool expectedArg = false;
        if (Str8StartsWith(node->val, STR8_LIT("-"), 0))
        {
            expectedArg = true;
            if (Str8StartsWith(arg, STR8_LIT("--"), 0))
            {
                arg = Str8Skip(arg, 2);
            }
            else
            {
                arg = Str8Skip(arg, 1);
            }
        }
        
        for (u64 i = 0; i < gBaseCmdlineArgDefs.len; i++)
        {
            CmdlineArgDef *def = gBaseCmdlineArgDefs.data + i;
            void *val = (def->value.kind == CMDLINE_ARG_REF) ? def->value._union.asRef : &def->value._union;

            if (Str8Equals(def->name, arg, 0))
            {
                switch (def->defValue.kind)
                {
                    case CMDLINE_ARG_I64:
                    {
                        if (node->next != null)
                        {
                            if (!I64TryFromStr8(node->next->val, (i64*)val))
                            {
                                baseEPrintf("{r}Expected integer argument after commandline arg '%S' instead got '%S'.\n", node->val, node->next->val);
                                result = result && false;
                            }
                        }
                        else
                        {
                            baseEPrintf("{r}Expected integer argument after commandline arg '%S'.\n", node->val);
                            result = result && false;
                        }

                        node = node->next;
                        break;
                    }break;

                    case CMDLINE_ARG_STR8:
                    {
                        if (node->next != null)
                        {
                            *(str8*)val = node->next->val;
                        }
                        else
                        {
                            baseEPrintf("{r}Expected string argument after commandline arg '%S'.\n", node->val);
                            result = result && false;
                        }

                        node = node->next;
                        break;
                    }break;

                    case CMDLINE_ARG_STR8LIST:
                    {
                        if (node->next != null)
                        {
                            Str8ListNode temp = *node->next;
                            node->next->next = null;
                            node->next->prev = null;
                            Str8ListPushNodeLast((Str8List*)val, node->next);

                            *node->next = temp;
                        }
                        else
                        {
                            baseEPrintf("{r}Expected string argument after commandline arg '%S'.\n", node->val);
                            result = result && false;
                        }

                        node = node->next;
                    }break;

                    case CMDLINE_ARG_BOOL:
                    {
                        *(bool*)val = true;
                    }break;

                    default:
                    {
                        baseEPrintf("{r}Unhandled commandline arg kind '%S'.\n", node->val);
                    }break;
                }

                def->passed = true;
                found = true;
                break;
            }
        }

        if (!found)
        {
            if (expectedArg)
            {
                baseEPrintf("{r}Commandline arg '%S' was passed but it's not a valid arg.\n", node->val);
                result = result && false;
            }
            else
            {
                Str8ListNode temp = *node;

                node->next = null;
                node->prev = null;
                Str8ListPushNodeLast(&gBaseCmdlineTrailingArgs, node);

                *node = temp;
            }
        }
    }

    for(u64 i = 0; i < gBaseCmdlineArgDefs.len; i++)
    {
        CmdlineArgDef *def = gBaseCmdlineArgDefs.data + i;

        if (!def->passed && def->presence == CMDLINE_ARG_PRESENCE_REQUIRED)
        {
            baseEPrintf("{r}Commandline arg '--%S' is required but was not passed.\n", def->name);
            result = result && false;
        }
    }

    return result;
}

Str8List *cmdlineTrailing(str8 help)
{
    gBaseCmdlineTrailingHelp = help;
    return &gBaseCmdlineTrailingArgs;
}

void cmdlineUsage(void)
{
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str8 programName = Str8ChopBefore(OSGetProgramPath(temp.arena), STR8_LIT("/"), STR_MATCHFLAGS_SLASH_INSENSITIVE | STR_MATCHFLAGS_FIND_LAST);
        
        basePrintf("Usage: %S <args> %S", programName, gBaseCmdlineTrailingHelp);

        if (gBaseCmdlineArgDefs.len > 0)
        {
            basePrintf("\nargs:", programName);
        }

        for(u64 i = 0; i < gBaseCmdlineArgDefs.len; i++)
        {
            CmdlineArgDef def = gBaseCmdlineArgDefs.data[i];
            basePrintf("\n    --%S/-%S/%S", def.name, def.name, def.name);
            if (def.presence == CMDLINE_ARG_PRESENCE_OPTIONAL)
            {
                basePrintf(" (optional)");
            }
            else
            {
                basePrintf(" (mandatory)");
            }
            basePrintf("\n");
            switch (def.defValue.kind)
            {
                case CMDLINE_ARG_I64:
                {
                    basePrintf("        default (integer): %lld\n        info: %S", def.defValue._union.asI64, def.help);
                }break;
                case CMDLINE_ARG_STR8:
                {
                    basePrintf("        default (string): %S\n        info: %S", def.defValue._union.asStr8, def.help);
                }break;
                case CMDLINE_ARG_STR8LIST:
                {
                    basePrintf("        default (string): %S\n        info: %S", Str8ListJoin(temp.arena, &def.defValue._union.asStr8list, &(Str8ListJoinParams){.pre = STR8_LIT("{"), .sep = STR8_LIT(","), .post = STR8_LIT("}")}), def.help);
                }break;
                case CMDLINE_ARG_BOOL:
                {
                    basePrintf("        default (boolean): %s\n        info: %S", def.defValue._union.asBool ? "true" : "false", def.help);
                }break;
                default:
                {
                    basePrintf("{r}unhandled commandline arg type", def.help);
                }break;
            }
        }
        basePrintf("\n", programName);
    }
    baseTempEnd(temp);
}

bool cmdlineStructEx(void *s, MetagenStructMembArray membs)
{
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        for(u64 i = 0; i < membs.len; i++)
        {
            MetagenStructMemb memb = membs.data[i];
            if (Str8StartsWith(memb.note, STR8("@cmdline "), 0))
            {
                str8 desc = Str8ChopBefore(memb.note, STR8("@cmdline "), 0);
                Str8List descParts = Str8Split(temp.arena, desc, STR8(", "), 0, STR_SPLITFLAGS_DISCARD_EMPTY | STR_SPLITFLAGS_NO_COPY);

                if (descParts.len != 3)
                {
                    baseEPrintf("{r}Failed to parse commandline args. Invalid note '%S' on member '%S'\n", memb.note, memb.name);
                    baseTempEnd(temp);
                    return false;
                }
                else
                {
                    str8 name = descParts.first->val;
                    str8 defVal = descParts.first->next->val;
                    str8 helpVal = descParts.first->next->next->val;

                    switch (memb.type)
                    {
                        case METAGEN_TYPE_i64:
                        {
                            cmdlineI64Var((i64*)(((u8*)s) + memb.offset), name, I64FromStr8(defVal), helpVal, 0);
                        }break;

                        case METAGEN_TYPE_bool:
                        {
                            cmdlineBoolVar((bool*)(((u8*)s) + memb.offset), name, false, helpVal, 0);
                        }break;

                        case METAGEN_TYPE_str8:
                        {
                            cmdlineStr8Var((str8*)(((u8*)s) + memb.offset), name, defVal, helpVal, 0);
                        }break;

                        default:
                        {
                            baseEPrintf("{r}Unregognised for struct member '%S' when parsing cmdline args\n", memb.name);
                            baseTempEnd(temp);
                            return false;
                        }break;
                    }
                }
            }
        }
    }

    baseTempEnd(temp);

    return true;
}
