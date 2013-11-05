#ifndef __JAVA_LANG_ENUM__
#define __JAVA_LANG_ENUM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Enum 8
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Exception
#define XMLVM_FORWARD_DECL_java_lang_Exception
XMLVM_FORWARD_DECL(java_lang_Exception)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Method
#define XMLVM_FORWARD_DECL_java_lang_reflect_Method
XMLVM_FORWARD_DECL(java_lang_reflect_Method)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_AccessController
#define XMLVM_FORWARD_DECL_java_security_AccessController
XMLVM_FORWARD_DECL(java_security_AccessController)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.lang.Enum
XMLVM_DEFINE_CLASS(java_lang_Enum, 7, XMLVM_ITABLE_SIZE_java_lang_Enum)

extern JAVA_OBJECT __CLASS_java_lang_Enum;
extern JAVA_OBJECT __CLASS_java_lang_Enum_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Enum_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Enum_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Enum
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Enum \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT name_; \
        JAVA_INT ordinal_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Enum \
    } java_lang_Enum

struct java_lang_Enum {
    __TIB_DEFINITION_java_lang_Enum* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Enum;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Enum
#define XMLVM_FORWARD_DECL_java_lang_Enum
typedef struct java_lang_Enum java_lang_Enum;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Enum 7
#define XMLVM_VTABLE_IDX_java_lang_Enum_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Enum_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Enum_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Enum_clone__ 0
#define XMLVM_VTABLE_IDX_java_lang_Enum_finalize_java_lang_Enum__ 2
#define XMLVM_VTABLE_IDX_java_lang_Enum_compareTo___java_lang_Object 6

void __INIT_java_lang_Enum();
void __INIT_IMPL_java_lang_Enum();
void __DELETE_java_lang_Enum(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Enum(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Enum();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Enum();
JAVA_LONG java_lang_Enum_GET_serialVersionUID();
void java_lang_Enum_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_Enum___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT java_lang_Enum_name__(JAVA_OBJECT me);
JAVA_INT java_lang_Enum_ordinal__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Enum_toString__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Enum_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_lang_Enum_hashCode__(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_lang_Enum_clone__(JAVA_OBJECT me);
JAVA_INT java_lang_Enum_compareTo___java_lang_Enum(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Enum_getDeclaringClass__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Enum_valueOf___java_lang_Class_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Enum_getValues___java_lang_Class(JAVA_OBJECT n1);
// Vtable index: 2
void java_lang_Enum_finalize_java_lang_Enum__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_INT java_lang_Enum_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
