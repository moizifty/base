#include "baseCmdLine.h"

u64 cmdlineGetHashOfOption(str8 option)
{
    return hashDJB2(option.data, option.len);
}
CmdLineHashMap cmdlineParseCmdLineFromStringList(BaseArena *arena, Str8List list)
{
    CmdLineHashMap ret = {0};
    ret.originalInputs = list;

    u64 slotsLen = list.len + 16;
    ret.slots.data = baseArenaPush(arena, sizeof(CmdLineOptSlot) * slotsLen);
    ret.slots.len = slotsLen;

    BASE_LIST_FOREACH(Str8ListNode, str, list)
    {
        u64 hash = cmdlineGetHashOfOption(str->val);
        u64 slotIndex = hash % slotsLen;
        
        CmdLineOptNode *optionNode = baseArenaPush(arena, sizeof(CmdLineOptNode));
        optionNode->option = str->val;
        
        BaseDllNodePushLast(ret.slots.data[slotIndex].first, ret.slots.data[slotIndex].last, optionNode);
    }

    return ret;
}
str8 cmdlineGetStr8(CmdLineHashMap *map, str8 option);
bool cmdlineGetFlag(CmdLineHashMap *map, str8 option)
{
    u64 hash = cmdlineGetHashOfOption(option) % map->slots.len;
    CmdLineOptSlot slot = map->slots.data[hash];

    BASE_LIST_FOREACH(CmdLineOptNode, optNode, slot)
    {
        if (baseStringsStrEquals(optNode->option, option, 0))
        {
            return true;
        }
    }

    return false;
}
i64 cmdlineGetI64(CmdLineHashMap *map, str8 option);