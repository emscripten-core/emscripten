#ifndef __JAVA_UTIL_MAPENTRY__
#define __JAVA_UTIL_MAPENTRY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_MapEntry 5
// Implemented interfaces:
#include "java_lang_Cloneable.h"
#include "java_util_Map_Entry.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for java.util.MapEntry
XMLVM_DEFINE_CLASS(java_util_MapEntry, 9, XMLVM_ITABLE_SIZE_java_util_MapEntry)

extern JAVA_OBJECT __CLASS_java_util_MapEntry;
extern JAVA_OBJECT __CLASS_java_util_MapEntry_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_MapEntry_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_MapEntry_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_MapEntry
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_MapEntry \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT key_; \
        JAVA_OBJECT value_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_MapEntry \
    } java_util_MapEntry

struct java_util_MapEntry {
    __TIB_DEFINITION_java_util_MapEntry* tib;
    struct {
        __INSTANCE_FIELDS_java_util_MapEntry;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_MapEntry
#define XMLVM_FORWARD_DECL_java_util_MapEntry
typedef struct java_util_MapEntry java_util_MapEntry;
#endif

#define XMLVM_VTABLE_SIZE_java_util_MapEntry 9
#define XMLVM_VTABLE_IDX_java_util_MapEntry_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_MapEntry_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_MapEntry_getKey__ 6
#define XMLVM_VTABLE_IDX_java_util_MapEntry_getValue__ 7
#define XMLVM_VTABLE_IDX_java_util_MapEntry_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_MapEntry_setValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_MapEntry_toString__ 5

void __INIT_java_util_MapEntry();
void __INIT_IMPL_java_util_MapEntry();
void __DELETE_java_util_MapEntry(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_MapEntry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_MapEntry();
JAVA_OBJECT __NEW_INSTANCE_java_util_MapEntry();
void java_util_MapEntry___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_MapEntry___INIT____java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 0
JAVA_OBJECT java_util_MapEntry_clone__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_util_MapEntry_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_OBJECT java_util_MapEntry_getKey__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_util_MapEntry_getValue__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_util_MapEntry_hashCode__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_OBJECT java_util_MapEntry_setValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_util_MapEntry_toString__(JAVA_OBJECT me);

#endif
