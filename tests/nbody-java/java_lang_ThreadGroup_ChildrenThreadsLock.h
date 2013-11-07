#ifndef __JAVA_LANG_THREADGROUP_CHILDRENTHREADSLOCK__
#define __JAVA_LANG_THREADGROUP_CHILDRENTHREADSLOCK__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ThreadGroup_ChildrenThreadsLock 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup
XMLVM_FORWARD_DECL(java_lang_ThreadGroup)
#endif
// Class declarations for java.lang.ThreadGroup$ChildrenThreadsLock
XMLVM_DEFINE_CLASS(java_lang_ThreadGroup_ChildrenThreadsLock, 6, XMLVM_ITABLE_SIZE_java_lang_ThreadGroup_ChildrenThreadsLock)

extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_ChildrenThreadsLock_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenThreadsLock
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenThreadsLock \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT this_0_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenThreadsLock \
    } java_lang_ThreadGroup_ChildrenThreadsLock

struct java_lang_ThreadGroup_ChildrenThreadsLock {
    __TIB_DEFINITION_java_lang_ThreadGroup_ChildrenThreadsLock* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ThreadGroup_ChildrenThreadsLock;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenThreadsLock
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenThreadsLock
typedef struct java_lang_ThreadGroup_ChildrenThreadsLock java_lang_ThreadGroup_ChildrenThreadsLock;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ThreadGroup_ChildrenThreadsLock 6

void __INIT_java_lang_ThreadGroup_ChildrenThreadsLock();
void __INIT_IMPL_java_lang_ThreadGroup_ChildrenThreadsLock();
void __DELETE_java_lang_ThreadGroup_ChildrenThreadsLock(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup_ChildrenThreadsLock(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ThreadGroup_ChildrenThreadsLock();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup_ChildrenThreadsLock();
void java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_ChildrenThreadsLock___INIT____java_lang_ThreadGroup_java_lang_ThreadGroup_1(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);

#endif
