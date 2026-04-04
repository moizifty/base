/**********************************************************************/
// GENERATED FILE
// Input: .\src\base\baseMath.h
// Date-Time: 28/3/2026 - 14:51
/**********************************************************************/

#include "baseMath.gen.h"

MetagenStructMembArray gvec2fMembDefsTable=
{
	.data=(MetagenStructMemb[2])
	{
		{.name = STR8_LIT_COMP_CONST("x"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("y"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=4,},
	},
	.len=2,
};
MetagenStruct gvec2fStructInfo=
{
   .name=STR8_LIT_COMP_CONST("vec2f"),
   .typeid=METAGEN_TYPE_vec2f,
   .size=8,
   .align=4,
   .membs=&gvec2fMembDefsTable,
};
MetagenStructMembArray gvec3fMembDefsTable=
{
	.data=(MetagenStructMemb[3])
	{
		{.name = STR8_LIT_COMP_CONST("x"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("y"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=4,},
		{.name = STR8_LIT_COMP_CONST("z"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=8,},
	},
	.len=3,
};
MetagenStruct gvec3fStructInfo=
{
   .name=STR8_LIT_COMP_CONST("vec3f"),
   .typeid=METAGEN_TYPE_vec3f,
   .size=12,
   .align=4,
   .membs=&gvec3fMembDefsTable,
};
MetagenStructMembArray grange3fMembDefsTable=
{
	.data=(MetagenStructMemb[10])
	{
		{.name = STR8_LIT_COMP_CONST("start"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_vec3f, .size=12, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("end"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_vec3f, .size=12, .offset=12,},
		{.name = STR8_LIT_COMP_CONST("min"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_vec3f, .size=12, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("max"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_vec3f, .size=12, .offset=12,},
		{.name = STR8_LIT_COMP_CONST("x0"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=0,},
		{.name = STR8_LIT_COMP_CONST("y0"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=4,},
		{.name = STR8_LIT_COMP_CONST("z0"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=8,},
		{.name = STR8_LIT_COMP_CONST("x1"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=12,},
		{.name = STR8_LIT_COMP_CONST("y1"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=16,},
		{.name = STR8_LIT_COMP_CONST("z1"), .note = STR8_LIT_COMP_CONST(""), .type = METAGEN_TYPE_f32, .size=4, .offset=20,},
	},
	.len=10,
};
MetagenStruct grange3fStructInfo=
{
   .name=STR8_LIT_COMP_CONST("range3f"),
   .typeid=METAGEN_TYPE_range3f,
   .size=24,
   .align=4,
   .membs=&grange3fMembDefsTable,
};
