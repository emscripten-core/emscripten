#ifndef __JAVA_LANG_OBJECT_ADDEDMEMBERS__
#define __JAVA_LANG_OBJECT_ADDEDMEMBERS__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Object_AddedMembers 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
XMLVM_FORWARD_DECL(java_lang_Thread)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_ArrayList
#define XMLVM_FORWARD_DECL_java_util_ArrayList
XMLVM_FORWARD_DECL(java_util_ArrayList)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_List
#define XMLVM_FORWARD_DECL_java_util_List
XMLVM_FORWARD_DECL(java_util_List)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Mutex)
#endif
// Class declarations for java.lang.Object$AddedMembers
XMLVM_DEFINE_CLASS(java_lang_Object_AddedMembers, 6, XMLVM_ITABLE_SIZE_java_lang_Object_AddedMembers)

extern JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers;
extern JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Object_AddedMembers_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Object_AddedMembers
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Object_AddedMembers \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_INT recursiveLocks_; \
        JAVA_OBJECT owningThread_; \
        JAVA_OBJECT instanceMutex_; \
        JAVA_OBJECT waitingConditions_; \
        JAVA_INT notifyAllMaxIndex_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Object_AddedMembers \
    } java_lang_Object_AddedMembers

struct java_lang_Object_AddedMembers {
    __TIB_DEFINITION_java_lang_Object_AddedMembers* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Object_AddedMembers;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Object_AddedMembers
#define XMLVM_FORWARD_DECL_java_lang_Object_AddedMembers
typedef struct java_lang_Object_AddedMembers java_lang_Object_AddedMembers;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Object_AddedMembers 6

void __INIT_java_lang_Object_AddedMembers();
void __INIT_IMPL_java_lang_Object_AddedMembers();
void __DELETE_java_lang_Object_AddedMembers(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Object_AddedMembers(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Object_AddedMembers();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Object_AddedMembers();
void java_lang_Object_AddedMembers___INIT___(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Object_AddedMembers_access$000___java_lang_Object_AddedMembers(JAVA_OBJECT n1);
JAVA_INT java_lang_Object_AddedMembers_access$100___java_lang_Object_AddedMembers(JAVA_OBJECT n1);
JAVA_INT java_lang_Object_AddedMembers_access$110___java_lang_Object_AddedMembers(JAVA_OBJECT n1);
JAVA_INT java_lang_Object_AddedMembers_access$102___java_lang_Object_AddedMembers_int(JAVA_OBJECT n1, JAVA_INT n2);

#endif
