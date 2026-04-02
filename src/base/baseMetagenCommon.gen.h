/**********************************************************************/
// GENERATED FILE
// Date-Time: 28/3/2026 - 14:51
/**********************************************************************/

extern MetagenStructMembArray gDateTimeMembDefsTable;
extern MetagenStructMembArray gvec2fMembDefsTable;
extern MetagenStructMembArray gvec3fMembDefsTable;
extern MetagenStructMembArray grange3fMembDefsTable;
#define METAGEN_TYPE_DateTime (METAGEN_TYPE_CUSTOM_BEGIN + 0)
#define METAGEN_TYPE_vec2f (METAGEN_TYPE_CUSTOM_BEGIN + 1)
#define METAGEN_TYPE_vec3f (METAGEN_TYPE_CUSTOM_BEGIN + 2)
#define METAGEN_TYPE_range3f (METAGEN_TYPE_CUSTOM_BEGIN + 3)
#define METAGEN_PRINT_MEMB_CUSTOM \
         case METAGEN_TYPE_DateTime: Str8ListPushLast(arena, list, StructToStr8(arena, (Any){.data = ((u8*)(member) + (size*i)), .info = gDateTimeStructInfo})); break;\
         case METAGEN_TYPE_vec2f: Str8ListPushLast(arena, list, StructToStr8(arena, (Any){.data = ((u8*)(member) + (size*i)), .info = gvec2fStructInfo})); break;\
         case METAGEN_TYPE_vec3f: Str8ListPushLast(arena, list, StructToStr8(arena, (Any){.data = ((u8*)(member) + (size*i)), .info = gvec3fStructInfo})); break;\
         case METAGEN_TYPE_range3f: Str8ListPushLast(arena, list, StructToStr8(arena, (Any){.data = ((u8*)(member) + (size*i)), .info = grange3fStructInfo})); break;\
