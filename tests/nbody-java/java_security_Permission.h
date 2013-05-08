#ifndef __JAVA_SECURITY_PERMISSION__
#define __JAVA_SECURITY_PERMISSION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_security_Permission 1
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_security_Guard.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_SecurityManager
#define XMLVM_FORWARD_DECL_java_lang_SecurityManager
XMLVM_FORWARD_DECL(java_lang_SecurityManager)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
// Class declarations for java.security.Permission
XMLVM_DEFINE_CLASS(java_security_Permission, 10, XMLVM_ITABLE_SIZE_java_security_Permission)

extern JAVA_OBJECT __CLASS_java_security_Permission;
extern JAVA_OBJECT __CLASS_java_security_Permission_1ARRAY;
extern JAVA_OBJECT __CLASS_java_security_Permission_2ARRAY;
extern JAVA_OBJECT __CLASS_java_security_Permission_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_security_Permission
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_security_Permission \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT name_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_security_Permission \
    } java_security_Permission

struct java_security_Permission {
    __TIB_DEFINITION_java_security_Permission* tib;
    struct {
        __INSTANCE_FIELDS_java_security_Permission;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_security_Permission
#define XMLVM_FORWARD_DECL_java_security_Permission
typedef struct java_security_Permission java_security_Permission;
#endif

#define XMLVM_VTABLE_SIZE_java_security_Permission 10
#define XMLVM_VTABLE_IDX_java_security_Permission_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_security_Permission_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_security_Permission_getActions__ 7
#define XMLVM_VTABLE_IDX_java_security_Permission_implies___java_security_Permission 8
#define XMLVM_VTABLE_IDX_java_security_Permission_checkGuard___java_lang_Object 6
#define XMLVM_VTABLE_IDX_java_security_Permission_newPermissionCollection__ 9
#define XMLVM_VTABLE_IDX_java_security_Permission_toString__ 5

void __INIT_java_security_Permission();
void __INIT_IMPL_java_security_Permission();
void __DELETE_java_security_Permission(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_security_Permission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_security_Permission();
JAVA_OBJECT __NEW_INSTANCE_java_security_Permission();
JAVA_LONG java_security_Permission_GET_serialVersionUID();
void java_security_Permission_PUT_serialVersionUID(JAVA_LONG v);
// Vtable index: 1
JAVA_BOOLEAN java_security_Permission_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_security_Permission_hashCode__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_security_Permission_getActions__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_security_Permission_implies___java_security_Permission(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_security_Permission___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_security_Permission_getName__(JAVA_OBJECT me);
// Vtable index: 6
void java_security_Permission_checkGuard___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_security_Permission_newPermissionCollection__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_security_Permission_toString__(JAVA_OBJECT me);

#endif
