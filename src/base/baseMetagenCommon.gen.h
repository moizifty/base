/**********************************************************************/
/* GENERATED FILE
/* Date-Time: 20/1/2025 - 14:21
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
         case METAGEN_TYPE_DateTime: basePrintStruct(((u8*)(member) + (size*i)), gDateTimeMembDefsTable); break;\
         case METAGEN_TYPE_vec2f: basePrintStruct(((u8*)(member) + (size*i)), gvec2fMembDefsTable); break;\
         case METAGEN_TYPE_vec3f: basePrintStruct(((u8*)(member) + (size*i)), gvec3fMembDefsTable); break;\
         case METAGEN_TYPE_range3f: basePrintStruct(((u8*)(member) + (size*i)), grange3fMembDefsTable); break;\
