#include "baseCmdLine.h"

u64 cmdlineGetHashOfOption(str8 option)
{
    return hashDJB2((i8 *)option.data, option.len);
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
        
        CmdLineOptSlot slot = ret.slots.data[slotIndex];
        BaseDllNodePushLast(slot.first, slot.last, optionNode);
    }

    return ret;
}
str8 cmdlineGetStr8(CmdLineHashMap *map, str8 option);
bool cmdlineGetFlag(CmdLineHashMap *map, str8 option);
i64 cmdlineGetI64(CmdLineHashMap *map, str8 option);