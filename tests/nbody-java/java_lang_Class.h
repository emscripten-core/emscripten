#ifndef __JAVA_LANG_CLASS__
#define __JAVA_LANG_CLASS__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Class 5
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_reflect_AnnotatedElement.h"
#include "java_lang_reflect_GenericDeclaration.h"
#include "java_lang_reflect_Type.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_File
#define XMLVM_FORWARD_DECL_java_io_File
XMLVM_FORWARD_DECL(java_io_File)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
XMLVM_FORWARD_DECL(java_io_IOException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassLoader
#define XMLVM_FORWARD_DECL_java_lang_ClassLoader
XMLVM_FORWARD_DECL(java_lang_ClassLoader)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_RuntimeException
#define XMLVM_FORWARD_DECL_java_lang_RuntimeException
XMLVM_FORWARD_DECL(java_lang_RuntimeException)
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
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Constructor
#define XMLVM_FORWARD_DECL_java_lang_reflect_Constructor
XMLVM_FORWARD_DECL(java_lang_reflect_Constructor)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Field
#define XMLVM_FORWARD_DECL_java_lang_reflect_Field
XMLVM_FORWARD_DECL(java_lang_reflect_Field)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_reflect_Method
#define XMLVM_FORWARD_DECL_java_lang_reflect_Method
XMLVM_FORWARD_DECL(java_lang_reflect_Method)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
XMLVM_FORWARD_DECL(org_xmlvm_runtime_XMLVMClassLoader)
#endif
// Class declarations for java.lang.Class
XMLVM_DEFINE_CLASS(java_lang_Class, 13, XMLVM_ITABLE_SIZE_java_lang_Class)

extern JAVA_OBJECT __CLASS_java_lang_Class;
extern JAVA_OBJECT __CLASS_java_lang_Class_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Class_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Class_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Class
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Class \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT tib_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Class \
    } java_lang_Class

struct java_lang_Class {
    __TIB_DEFINITION_java_lang_Class* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Class;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
typedef struct java_lang_Class java_lang_Class;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Class 13
#define XMLVM_VTABLE_IDX_java_lang_Class_getAnnotation___java_lang_Class 6
#define XMLVM_VTABLE_IDX_java_lang_Class_getAnnotations__ 7
#define XMLVM_VTABLE_IDX_java_lang_Class_getDeclaredAnnotations__ 8
#define XMLVM_VTABLE_IDX_java_lang_Class_getMethod___java_lang_String_java_lang_Class_1ARRAY 9
#define XMLVM_VTABLE_IDX_java_lang_Class_getTypeParameters__ 10
#define XMLVM_VTABLE_IDX_java_lang_Class_isAnnotationPresent___java_lang_Class 11
#define XMLVM_VTABLE_IDX_java_lang_Class_newInstance__ 12
#define XMLVM_VTABLE_IDX_java_lang_Class_toString__ 5

void __INIT_java_lang_Class();
void __INIT_IMPL_java_lang_Class();
void __DELETE_java_lang_Class(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Class(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Class();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Class();
void xmlvm_init_native_java_lang_Class();
JAVA_LONG java_lang_Class_GET_serialVersionUID();
void java_lang_Class_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_lang_Class_GET_classLoader();
void java_lang_Class_PUT_classLoader(JAVA_OBJECT v);
void java_lang_Class_initNativeLayer__();
void java_lang_Class___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getStackClasses___int_boolean(JAVA_INT n1, JAVA_BOOLEAN n2);
JAVA_OBJECT java_lang_Class_forName___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_OBJECT n3);
JAVA_OBJECT java_lang_Class_getClasses__(JAVA_OBJECT me);
void java_lang_Class_verify__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT java_lang_Class_getAnnotation___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_OBJECT java_lang_Class_getAnnotations__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getCanonicalName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getClassLoader__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getClassLoaderImpl__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getComponentType__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getConstructor___java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getConstructors__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_OBJECT java_lang_Class_getDeclaredAnnotations__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getDeclaredClasses__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getDeclaredConstructor___java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Class_getDeclaredConstructors__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getDeclaredField___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getDeclaredFields__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getDeclaredMethod___java_lang_String_java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Class_getDeclaredMethods__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getDeclaringClass__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getEnclosingClass__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getEnclosingConstructor__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getEnclosingMethod__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getEnumConstants__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getField___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getFields__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getGenericInterfaces__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getGenericSuperclass__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getInterfaces__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_OBJECT java_lang_Class_getMethod___java_lang_String_java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Class_getMethods__(JAVA_OBJECT me);
JAVA_INT java_lang_Class_getModifiers__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getSimpleName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getProtectionDomain__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getPDImpl__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getResourceAsStream___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getSigners__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getSuperclass__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_OBJECT java_lang_Class_getTypeParameters__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isAnnotation__(JAVA_OBJECT me);
// Vtable index: 11
JAVA_BOOLEAN java_lang_Class_isAnnotationPresent___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Class_isAnonymousClass__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isArray__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isAssignableFrom___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Class_isEnum__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isInstance___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Class_isInterface__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isLocalClass__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isMemberClass__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isPrimitive__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_isSynthetic__(JAVA_OBJECT me);
// Vtable index: 12
JAVA_OBJECT java_lang_Class_newInstance__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Class_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getPackage__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_getPackageString__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Class_desiredAssertionStatus__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Class_asSubclass___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_cast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Class_getSignatureAttribute__(JAVA_OBJECT me);
void java_lang_Class___CLINIT_();

#endif
