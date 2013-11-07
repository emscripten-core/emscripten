#ifndef __JAVA_UTIL_PROPERTYPERMISSION__
#define __JAVA_UTIL_PROPERTYPERMISSION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_PropertyPermission 1
// Implemented interfaces:
// Super Class:
#include "java_security_BasicPermission.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamField
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamField
XMLVM_FORWARD_DECL(java_io_ObjectStreamField)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_Permission
#define XMLVM_FORWARD_DECL_java_security_Permission
XMLVM_FORWARD_DECL(java_security_Permission)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_StringTokenizer
#define XMLVM_FORWARD_DECL_java_util_StringTokenizer
XMLVM_FORWARD_DECL(java_util_StringTokenizer)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_Util
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_Util
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_Util)
#endif
// Class declarations for java.util.PropertyPermission
XMLVM_DEFINE_CLASS(java_util_PropertyPermission, 10, XMLVM_ITABLE_SIZE_java_util_PropertyPermission)

extern JAVA_OBJECT __CLASS_java_util_PropertyPermission;
extern JAVA_OBJECT __CLASS_java_util_PropertyPermission_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_PropertyPermission_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_PropertyPermission_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_PropertyPermission
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_PropertyPermission \
    __INSTANCE_FIELDS_java_security_BasicPermission; \
    struct { \
        JAVA_BOOLEAN read_; \
        JAVA_BOOLEAN write_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_PropertyPermission \
    } java_util_PropertyPermission

struct java_util_PropertyPermission {
    __TIB_DEFINITION_java_util_PropertyPermission* tib;
    struct {
        __INSTANCE_FIELDS_java_util_PropertyPermission;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_PropertyPermission
#define XMLVM_FORWARD_DECL_java_util_PropertyPermission
typedef struct java_util_PropertyPermission java_util_PropertyPermission;
#endif

#define XMLVM_VTABLE_SIZE_java_util_PropertyPermission 10
#define XMLVM_VTABLE_IDX_java_util_PropertyPermission_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_util_PropertyPermission_getActions__ 7
#define XMLVM_VTABLE_IDX_java_util_PropertyPermission_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_util_PropertyPermission_implies___java_security_Permission 8
#define XMLVM_VTABLE_IDX_java_util_PropertyPermission_newPermissionCollection__ 9

void __INIT_java_util_PropertyPermission();
void __INIT_IMPL_java_util_PropertyPermission();
void __DELETE_java_util_PropertyPermission(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_PropertyPermission(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_PropertyPermission();
JAVA_OBJECT __NEW_INSTANCE_java_util_PropertyPermission();
JAVA_LONG java_util_PropertyPermission_GET_serialVersionUID();
void java_util_PropertyPermission_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_util_PropertyPermission_GET_serialPersistentFields();
void java_util_PropertyPermission_PUT_serialPersistentFields(JAVA_OBJECT v);
void java_util_PropertyPermission___CLINIT_();
void java_util_PropertyPermission___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_PropertyPermission_decodeActions___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 1
JAVA_BOOLEAN java_util_PropertyPermission_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_OBJECT java_util_PropertyPermission_getActions__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_util_PropertyPermission_hashCode__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_util_PropertyPermission_implies___java_security_Permission(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 9
JAVA_OBJECT java_util_PropertyPermission_newPermissionCollection__(JAVA_OBJECT me);
void java_util_PropertyPermission_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_PropertyPermission_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
