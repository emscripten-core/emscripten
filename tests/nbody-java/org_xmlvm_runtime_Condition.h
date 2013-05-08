#ifndef __ORG_XMLVM_RUNTIME_CONDITION__
#define __ORG_XMLVM_RUNTIME_CONDITION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_Condition 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Mutex)
#endif
// Class declarations for org.xmlvm.runtime.Condition
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_Condition, 6, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_Condition)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Condition_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_Condition
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_Condition \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT pthreadCondT_; \
        JAVA_OBJECT synchronizedObject_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_Condition \
    } org_xmlvm_runtime_Condition

struct org_xmlvm_runtime_Condition {
    __TIB_DEFINITION_org_xmlvm_runtime_Condition* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_Condition;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
typedef struct org_xmlvm_runtime_Condition org_xmlvm_runtime_Condition;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_Condition 6

void __INIT_org_xmlvm_runtime_Condition();
void __INIT_IMPL_org_xmlvm_runtime_Condition();
void __DELETE_org_xmlvm_runtime_Condition(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Condition(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_Condition();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_Condition();
void xmlvm_init_native_org_xmlvm_runtime_Condition();
void org_xmlvm_runtime_Condition___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void org_xmlvm_runtime_Condition_initNativeConditionInstance__(JAVA_OBJECT me);
JAVA_OBJECT org_xmlvm_runtime_Condition_getSynchronizedObject__(JAVA_OBJECT me);
void org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2);
void org_xmlvm_runtime_Condition_signal__(JAVA_OBJECT me);
void org_xmlvm_runtime_Condition_broadcast__(JAVA_OBJECT me);

#endif
