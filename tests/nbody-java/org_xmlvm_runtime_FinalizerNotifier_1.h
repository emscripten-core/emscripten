#ifndef __ORG_XMLVM_RUNTIME_FINALIZERNOTIFIER_1__
#define __ORG_XMLVM_RUNTIME_FINALIZERNOTIFIER_1__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_FinalizerNotifier_1 1
// Implemented interfaces:
// Super Class:
#include "java_lang_Thread.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Condition)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_FinalizerNotifier
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_FinalizerNotifier
XMLVM_FORWARD_DECL(org_xmlvm_runtime_FinalizerNotifier)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Mutex)
#endif
// Class declarations for org.xmlvm.runtime.FinalizerNotifier$1
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_FinalizerNotifier_1, 7, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_FinalizerNotifier_1)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_FinalizerNotifier_1_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_FinalizerNotifier_1
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_FinalizerNotifier_1 \
    __INSTANCE_FIELDS_java_lang_Thread; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_FinalizerNotifier_1 \
    } org_xmlvm_runtime_FinalizerNotifier_1

struct org_xmlvm_runtime_FinalizerNotifier_1 {
    __TIB_DEFINITION_org_xmlvm_runtime_FinalizerNotifier_1* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_FinalizerNotifier_1;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_FinalizerNotifier_1
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_FinalizerNotifier_1
typedef struct org_xmlvm_runtime_FinalizerNotifier_1 org_xmlvm_runtime_FinalizerNotifier_1;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_FinalizerNotifier_1 7
#define XMLVM_VTABLE_IDX_org_xmlvm_runtime_FinalizerNotifier_1_run__ 6

void __INIT_org_xmlvm_runtime_FinalizerNotifier_1();
void __INIT_IMPL_org_xmlvm_runtime_FinalizerNotifier_1();
void __DELETE_org_xmlvm_runtime_FinalizerNotifier_1(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_FinalizerNotifier_1(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_FinalizerNotifier_1();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_FinalizerNotifier_1();
void org_xmlvm_runtime_FinalizerNotifier_1___INIT___(JAVA_OBJECT me);
// Vtable index: 6
void org_xmlvm_runtime_FinalizerNotifier_1_run__(JAVA_OBJECT me);

#endif
