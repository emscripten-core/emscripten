#ifndef __JAVA_LANG_THREADGROUP_CHILDRENGROUPSLOCK__
#define __JAVA_LANG_THREADGROUP_CHILDRENGROUPSLOCK__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ThreadGroup_ChildrenGroupsLock 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup
XMLVM_FORWARD_DECL(java_lang_ThreadGroup)
#endif
// Class declarations for java.lang.ThreadGroup$ChildrenGroupsLock
XMLVM_DEFINE_CLASS(java_lang_ThreadGroup_ChildrenGroupsLock, 6, XMLVM_ITABLE_SIZE_java_lang_ThreadGroup_ChildrenGroupsLock)

extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenGroupsLock_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenGroupsLock
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenGroupsLock \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT this_0_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenGroupsLock \
    } java_lang_ThreadGroup_ChildrenGroupsLock

struct java_lang_ThreadGroup_ChildrenGroupsLock {
    __TIB_DEFINITION_java_lang_ThreadGroup_ChildrenGroupsLock* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenGroupsLock;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenGroupsLock
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenGroupsLock
typedef struct java_lang_ThreadGroup_ChildrenGroupsLock java_lang_ThreadGroup_ChildrenGroupsLock;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ThreadGroup_ChildrenGroupsLock 6

void __INIT_java_lang_ThreadGroup_ChildrenGroupsLock();
void __INIT_IMPL_java_lang_ThreadGroup_ChildrenGroupsLock();
void __DELETE_java_lang_ThreadGroup_ChildrenGroupsLock(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenGroupsLock(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ThreadGroup_ChildrenGroupsLock();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenGroupsLock();
void java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_ChildrenGroupsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);

#endif
