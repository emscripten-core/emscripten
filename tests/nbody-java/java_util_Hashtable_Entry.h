#ifndef __JAVA_UTIL_HASHTABLE_ENTRY__
#define __JAVA_UTIL_HASHTABLE_ENTRY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Hashtable_Entry 5
// Implemented interfaces:
// Super Class:
#include "java_util_MapEntry.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for java.util.Hashtable$Entry
XMLVM_DEFINE_CLASS(java_util_Hashtable_Entry, 9, XMLVM_ITABLE_SIZE_java_util_Hashtable_Entry)

extern JAVA_OBJECT __CLASS_java_util_Hashtable_Entry;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Hashtable_Entry_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable_Entry
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Hashtable_Entry \
    __INSTANCE_FIELDS_java_util_MapEntry; \
    struct { \
        JAVA_OBJECT next_; \
        JAVA_INT hashcode_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Hashtable_Entry \
    } java_util_Hashtable_Entry

struct java_util_Hashtable_Entry {
    __TIB_DEFINITION_java_util_Hashtable_Entry* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Hashtable_Entry;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Hashtable_Entry
#define XMLVM_FORWARD_DECL_java_util_Hashtable_Entry
typedef struct java_util_Hashtable_Entry java_util_Hashtable_Entry;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Hashtable_Entry 9
#define XMLVM_VTABLE_IDX_java_util_Hashtable_Entry_clone__ 0
#define XMLVM_VTABLE_IDX_java_util_Hashtable_Entry_setValue___java_lang_Object 8
#define XMLVM_VTABLE_IDX_java_util_Hashtable_Entry_toString__ 5

void __INIT_java_util_Hashtable_Entry();
void __INIT_IMPL_java_util_Hashtable_Entry();
void __DELETE_java_util_Hashtable_Entry(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Hashtable_Entry(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Hashtable_Entry();
JAVA_OBJECT __NEW_INSTANCE_java_util_Hashtable_Entry();
void java_util_Hashtable_Entry___INIT____java_lang_Object_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 0
JAVA_OBJECT java_util_Hashtable_Entry_clone__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_OBJECT java_util_Hashtable_Entry_setValue___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_util_Hashtable_Entry_getKeyHash__(JAVA_OBJECT me);
JAVA_BOOLEAN java_util_Hashtable_Entry_equalsKey___java_lang_Object_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
// Vtable index: 5
JAVA_OBJECT java_util_Hashtable_Entry_toString__(JAVA_OBJECT me);

#endif
