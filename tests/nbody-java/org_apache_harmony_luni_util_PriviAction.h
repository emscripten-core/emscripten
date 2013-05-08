#ifndef __ORG_APACHE_HARMONY_LUNI_UTIL_PRIVIACTION__
#define __ORG_APACHE_HARMONY_LUNI_UTIL_PRIVIACTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_apache_harmony_luni_util_PriviAction 1
// Implemented interfaces:
#include "java_security_PrivilegedAction.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_AccessibleObject
#define XMLVM_FORWARD_DECL_java_lang_reflect_AccessibleObject
XMLVM_FORWARD_DECL(java_lang_reflect_AccessibleObject)
#endif
// Class declarations for org.apache.harmony.luni.util.PriviAction
XMLVM_DEFINE_CLASS(org_apache_harmony_luni_util_PriviAction, 7, XMLVM_ITABLE_SIZE_org_apache_harmony_luni_util_PriviAction)

extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_1ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_2ARRAY;
extern JAVA_OBJECT __CLASS_org_apache_harmony_luni_util_PriviAction_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_util_PriviAction
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_apache_harmony_luni_util_PriviAction \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT arg1_; \
        JAVA_OBJECT arg2_; \
        JAVA_INT action_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_apache_harmony_luni_util_PriviAction \
    } org_apache_harmony_luni_util_PriviAction

struct org_apache_harmony_luni_util_PriviAction {
    __TIB_DEFINITION_org_apache_harmony_luni_util_PriviAction* tib;
    struct {
        __INSTANCE_FIELDS_org_apache_harmony_luni_util_PriviAction;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
typedef struct org_apache_harmony_luni_util_PriviAction org_apache_harmony_luni_util_PriviAction;
#endif

#define XMLVM_VTABLE_SIZE_org_apache_harmony_luni_util_PriviAction 7
#define XMLVM_VTABLE_IDX_org_apache_harmony_luni_util_PriviAction_run__ 6

void __INIT_org_apache_harmony_luni_util_PriviAction();
void __INIT_IMPL_org_apache_harmony_luni_util_PriviAction();
void __DELETE_org_apache_harmony_luni_util_PriviAction(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_util_PriviAction(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_apache_harmony_luni_util_PriviAction();
JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_util_PriviAction();
JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SYSTEM_PROPERTY();
void org_apache_harmony_luni_util_PriviAction_PUT_GET_SYSTEM_PROPERTY(JAVA_INT v);
JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SECURITY_POLICY();
void org_apache_harmony_luni_util_PriviAction_PUT_GET_SECURITY_POLICY(JAVA_INT v);
JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_SET_ACCESSIBLE();
void org_apache_harmony_luni_util_PriviAction_PUT_SET_ACCESSIBLE(JAVA_INT v);
JAVA_INT org_apache_harmony_luni_util_PriviAction_GET_GET_SECURITY_PROPERTY();
void org_apache_harmony_luni_util_PriviAction_PUT_GET_SECURITY_PROPERTY(JAVA_INT v);
JAVA_OBJECT org_apache_harmony_luni_util_PriviAction_getSecurityProperty___java_lang_String(JAVA_OBJECT n1);
void org_apache_harmony_luni_util_PriviAction___INIT____int_java_lang_Object(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2);
void org_apache_harmony_luni_util_PriviAction___INIT___(JAVA_OBJECT me);
void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_reflect_AccessibleObject(JAVA_OBJECT me, JAVA_OBJECT n1);
void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 6
JAVA_OBJECT org_apache_harmony_luni_util_PriviAction_run__(JAVA_OBJECT me);

#endif
