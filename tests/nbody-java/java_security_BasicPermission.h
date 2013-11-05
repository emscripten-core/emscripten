#ifndef __JAVA_SECURITY_BASICPERMISSION__
#define __JAVA_SECURITY_BASICPERMISSION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_security_BasicPermission 1
// Implemented interfaces:
#include "java_io_Serializable.h"
// Super Class:
#include "java_security_Permission.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_RuntimePermission
#define XMLVM_FORWARD_DECL_java_lang_RuntimePermission
XMLVM_FORWARD_DECL(java_lang_RuntimePermission)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.security.BasicPermission
XMLVM_DEFINE_CLASS(java_security_BasicPermission, 10, XMLVM_ITABLE_SIZE_java_security_BasicPermission)

extern JAVA_OBJECT __CLASS_java_security_BasicPermission;
extern JAVA_OBJECT __CLASS_java_security_BasicPermission_1ARRAY;
extern JAVA_OBJECT __CLASS_java_security_BasicPermission_2ARRAY;
extern JAVA_OBJECT __CLASS_java_security_BasicPermission_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_security_BasicPermission
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_security_BasicPermission \
    __INSTANCE_FIELDS_java_security_Permission; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_security_BasicPermission \
    } java_security_BasicPermission

struct java_security_BasicPermission {
    __TIB_DEFINITION_java_security_BasicPermission* tib;
    struct {
        __INSTANCE_FIELDS_java_security_BasicPermission;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_security_BasicPermission
#define XMLVM_FORWARD_DECL_java_security_BasicPermission
typedef struct java_security_BasicPermission java_security_BasicPermission;
#endif

#define XMLVM_VTABLE_SIZE_java_security_BasicPermission 10
#define XMLVM_VTABLE_IDX_java_security_BasicPermission_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_security_BasicPermission_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_security_BasicPermission_getActions__ 7
#define XMLVM_VTABLE_IDX_java_security_BasicPermission_implies___java_security_Permission 8
#define XMLVM_VTABLE_IDX_java_security_BasicPermission_newPermissionCollection__ 9

void __INIT_java_security_BasicPermission();
void __INIT_IMPL_java_security_BasicPermission();
void __DELETE_java_security_BasicPermission(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_security_BasicPermission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_security_BasicPermission();
JAVA_OBJECT __NEW_INSTANCE_java_security_BasicPermission();
JAVA_LONG java_security_BasicPermission_GET_serialVersionUID();
void java_security_BasicPermission_PUT_serialVersionUID(JAVA_LONG v);
void java_security_BasicPermission___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_security_BasicPermission___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_security_BasicPermission_checkName___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 1
JAVA_BOOLEAN java_security_BasicPermission_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_security_BasicPermission_hashCode__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_security_BasicPermission_getActions__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_security_BasicPermission_implies___java_security_Permission(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_security_BasicPermission_nameImplies___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 9
JAVA_OBJECT java_security_BasicPermission_newPermissionCollection__(JAVA_OBJECT me);
void java_security_BasicPermission_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
