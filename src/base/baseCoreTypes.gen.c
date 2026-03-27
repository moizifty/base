/**********************************************************************/
// GENERATED FILE
// Input: .\src\base\baseCoreTypes.h
// Date-Time: 27/3/2026 - 16:52
/**********************************************************************/

#include "baseCoreTypes.gen.h"

extern MetagenStructMembArray gDateTimeMembDefsTable=
{
	.data=(MetagenStructMemb[8])
	{
		{.name = STR8_LIT_COMP_CONST("year"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u16, .size=2, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("month"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=2,},
		{.name = STR8_LIT_COMP_CONST("dayOfWeek"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=3,},
		{.name = STR8_LIT_COMP_CONST("day"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=4,},
		{.name = STR8_LIT_COMP_CONST("hour"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=5,},
		{.name = STR8_LIT_COMP_CONST("min"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=6,},
		{.name = STR8_LIT_COMP_CONST("sec"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u8, .size=1, .offset=7,},
		{.name = STR8_LIT_COMP_CONST("milli"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_u16, .size=2, .offset=8,},
	},
	.len=8,
};
MetagenStruct gDateTimeStructInfo=
{
   .name=STR8_LIT_COMP_CONST("DateTime"),
   .typeid=METAGEN_TYPE_DateTime,
   .size=10,
   .align=2,
   .membs=&gDateTimeMembDefsTable,
};
