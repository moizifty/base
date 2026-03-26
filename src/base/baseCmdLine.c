#include "baseCmdLine.h"
CmdlineArgDefArray gBaseCmdlineArgDefs = 
{
    .data = (CmdlineArgDef[BASE_CMDLINE_MAX_ARG_DEF_COUNT]){0},
    .len = 0,
};

Str8List gBaseCmdlineTrailingArgs = {0};

i64 *cmdlineI64(str8 name, i64 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->asI64 = def;
    nextDef->kind = CMDLINE_ARG_I64;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->asI64;
}
bool *cmdlineBool(str8 name, bool def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->asBool = def;
    nextDef->kind = CMDLINE_ARG_BOOL;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->asBool;
}
str8 *cmdlineStr8(str8 name, str8 def, str8 help, CmdlineArgPresenceKind presence)
{
    CmdlineArgDef *nextDef = gBaseCmdlineArgDefs.data + gBaseCmdlineArgDefs.len++;
    nextDef->name = name;
    nextDef->asStr8 = def;
    nextDef->kind = CMDLINE_ARG_STR8;
    nextDef->presence = presence;
    nextDef->passed = false;
    nextDef->help = help;

    return &nextDef->asStr8;
}
bool cmdlineParse(Str8List cmdline)
{
    if (!BASE_ANY(gBaseCmdlineArgDefs) && BASE_ANY(cmdline))
    {
        baseEPrintf("{r}No arguments are required by the program but some are passed.\n");
        return false;
    }
    
    bool result = true;
    BASE_LIST_FOREACH(Str8ListNode, node, cmdline)
    {
        if (Str8StartsWith(node->val, STR8_LIT("-"), 0))
        {
            str8 arg = node->val;
            if (Str8StartsWith(arg, STR8_LIT("--"), 0))
            {
                arg = Str8Skip(arg, 2);
            }
            else
            {
                arg = Str8Skip(arg, 1);
            }

            bool found = false;
            for (u64 i = 0; i < gBaseCmdlineArgDefs.len; i++)
            {
                CmdlineArgDef *def = gBaseCmdlineArgDefs.data + i;
                if (Str8Equals(def->name, arg, 0))
                {
                    switch (def->kind)
                    {
                        case CMDLINE_ARG_I64:
                        {
                            if (node->next != null)
                            {
                                if (!I64TryFromStr8(node->next->val, &def->asI64))
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
                                def->asStr8 = node->next->val;
                            }
                            else
                            {
                                baseEPrintf("{r}Expected string argument after commandline arg '%S'.\n", node->val);
                                result = result && false;
                            }

                            node = node->next;
                            break;
                        }break;

                        case CMDLINE_ARG_BOOL:
                        {
                            def->asBool = true;
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

            if(!found)
            {
                baseEPrintf("{r}Commandline arg '%S' was passed but it's not a valid arg.\n", node->val);
                result = result && false;
            }
        }
        else
        {
            Str8ListNode *temp = node;

            node->next = null;
            node->prev = null;
            Str8ListPushNodeLast(&gBaseCmdlineTrailingArgs, node);

            node = temp;
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

Str8List *cmdlineTrailing(void)
{
    return &gBaseCmdlineTrailingArgs;
}