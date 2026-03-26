/**********************************************************************/
// GENERATED FILE
// Date-Time: 26/3/2026 - 21:52
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
         case METAGEN_TYPE_DateTime: basePrintStructEx(((u8*)(member) + (size*i)), gDateTimeMembDefsTable); break;\
         case METAGEN_TYPE_vec2f: basePrintStructEx(((u8*)(member) + (size*i)), gvec2fMembDefsTable); break;\
         case METAGEN_TYPE_vec3f: basePrintStructEx(((u8*)(member) + (size*i)), gvec3fMembDefsTable); break;\
         case METAGEN_TYPE_range3f: basePrintStructEx(((u8*)(member) + (size*i)), grange3fMembDefsTable); break;\
