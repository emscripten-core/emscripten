#ifndef __JAVA_LANG_REFLECT_METHOD__
#define __JAVA_LANG_REFLECT_METHOD__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_reflect_Method 9
// Implemented interfaces:
#include "java_lang_reflect_GenericDeclaration.h"
#include "java_lang_reflect_Member.h"
// Super Class:
#include "java_lang_reflect_AccessibleObject.h"

// Circular references:
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
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Type
#define XMLVM_FORWARD_DECL_java_lang_reflect_Type
XMLVM_FORWARD_DECL(java_lang_reflect_Type)
#endif
// Class declarations for java.lang.reflect.Method
XMLVM_DEFINE_CLASS(java_lang_reflect_Method, 17, XMLVM_ITABLE_SIZE_java_lang_reflect_Method)

extern JAVA_OBJECT __CLASS_java_lang_reflect_Method;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Method_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Method_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_reflect_Method_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Method
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_reflect_Method \
    __INSTANCE_FIELDS_java_lang_reflect_AccessibleObject; \
    struct { \
        JAVA_OBJECT name_; \
        JAVA_OBJECT clazz_; \
        JAVA_OBJECT address_; \
        JAVA_INT slot_; \
        JAVA_OBJECT parameterTypes_; \
        JAVA_OBJECT exceptionTypes_; \
        JAVA_INT modifiers_; \
        JAVA_OBJECT signature_; \
        JAVA_OBJECT annotations_; \
        JAVA_OBJECT parameterAnnotations_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_reflect_Method \
    } java_lang_reflect_Method

struct java_lang_reflect_Method {
    __TIB_DEFINITION_java_lang_reflect_Method* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_reflect_Method;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Method
#define XMLVM_FORWARD_DECL_java_lang_reflect_Method
typedef struct java_lang_reflect_Method java_lang_reflect_Method;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_reflect_Method 17
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_getTypeParameters__ 14
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_isSynthetic__ 16
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_getDeclaringClass__ 12
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_getModifiers__ 9
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_getName__ 13
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY 15
#define XMLVM_VTABLE_IDX_java_lang_reflect_Method_toString__ 5

void __INIT_java_lang_reflect_Method();
void __INIT_IMPL_java_lang_reflect_Method();
void __DELETE_java_lang_reflect_Method(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Method(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_reflect_Method();
JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Method();
void xmlvm_init_native_java_lang_reflect_Method();
void java_lang_reflect_Method___INIT___(JAVA_OBJECT me);
void java_lang_reflect_Method___INIT____java_lang_String_java_lang_Class_java_lang_Class_1ARRAY_java_lang_Class_1ARRAY_int_java_lang_Object_int_java_lang_String_byte_1ARRAY_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_INT n5, JAVA_OBJECT n6, JAVA_INT n7, JAVA_OBJECT n8, JAVA_OBJECT n9, JAVA_OBJECT n10);
// Vtable index: 14
JAVA_OBJECT java_lang_reflect_Method_getTypeParameters__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_toGenericString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getGenericParameterTypes__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getGenericExceptionTypes__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getGenericReturnType__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getParameterAnnotations__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_reflect_Method_isVarArgs__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_reflect_Method_isBridge__(JAVA_OBJECT me);
// Vtable index: 16
JAVA_BOOLEAN java_lang_reflect_Method_isSynthetic__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getDefaultValue__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_reflect_Method_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 12
JAVA_OBJECT java_lang_reflect_Method_getDeclaringClass__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getExceptionTypes__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_INT java_lang_reflect_Method_getModifiers__(JAVA_OBJECT me);
// Vtable index: 13
JAVA_OBJECT java_lang_reflect_Method_getName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getParameterTypes__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_reflect_Method_getReturnType__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_reflect_Method_hashCode__(JAVA_OBJECT me);
// Vtable index: 15
JAVA_OBJECT java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 5
JAVA_OBJECT java_lang_reflect_Method_toString__(JAVA_OBJECT me);

#endif
