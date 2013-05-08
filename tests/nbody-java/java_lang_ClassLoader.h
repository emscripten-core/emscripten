#ifndef __JAVA_LANG_CLASSLOADER__
#define __JAVA_LANG_CLASSLOADER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ClassLoader 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ByteBuffer
XMLVM_FORWARD_DECL(java_nio_ByteBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
XMLVM_FORWARD_DECL(org_xmlvm_runtime_XMLVMClassLoader)
#endif
// Class declarations for java.lang.ClassLoader
XMLVM_DEFINE_CLASS(java_lang_ClassLoader, 8, XMLVM_ITABLE_SIZE_java_lang_ClassLoader)

extern JAVA_OBJECT __CLASS_java_lang_ClassLoader;
extern JAVA_OBJECT __CLASS_java_lang_ClassLoader_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ClassLoader_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ClassLoader_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ClassLoader
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ClassLoader \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ClassLoader \
    } java_lang_ClassLoader

struct java_lang_ClassLoader {
    __TIB_DEFINITION_java_lang_ClassLoader* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ClassLoader;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassLoader
#define XMLVM_FORWARD_DECL_java_lang_ClassLoader
typedef struct java_lang_ClassLoader java_lang_ClassLoader;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ClassLoader 8
#define XMLVM_VTABLE_IDX_java_lang_ClassLoader_findClass___java_lang_String 6
#define XMLVM_VTABLE_IDX_java_lang_ClassLoader_findResource___java_lang_String 7

void __INIT_java_lang_ClassLoader();
void __INIT_IMPL_java_lang_ClassLoader();
void __DELETE_java_lang_ClassLoader(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ClassLoader(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ClassLoader();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ClassLoader();
JAVA_OBJECT java_lang_ClassLoader_GET_systemClassLoader();
void java_lang_ClassLoader_PUT_systemClassLoader(JAVA_OBJECT v);
void java_lang_ClassLoader_initializeClassLoaders__();
JAVA_OBJECT java_lang_ClassLoader_getSystemClassLoader__();
JAVA_OBJECT java_lang_ClassLoader_getSystemResource___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getSystemResources___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getSystemResourceAsStream___java_lang_String(JAVA_OBJECT n1);
void java_lang_ClassLoader___INIT___(JAVA_OBJECT me);
void java_lang_ClassLoader___INIT____java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_defineClass___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
JAVA_OBJECT java_lang_ClassLoader_defineClass___java_lang_String_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4);
JAVA_OBJECT java_lang_ClassLoader_defineClass___java_lang_String_byte_1ARRAY_int_int_java_security_ProtectionDomain(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4, JAVA_OBJECT n5);
JAVA_OBJECT java_lang_ClassLoader_defineClass___java_lang_String_java_nio_ByteBuffer_java_security_ProtectionDomain(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
// Vtable index: 6
JAVA_OBJECT java_lang_ClassLoader_findClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_findLoadedClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_findSystemClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getParent__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ClassLoader_getResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getResources___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getResourceAsStream___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_loadClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_loadClass___java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_lang_ClassLoader_resolveClass___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_ClassLoader_isSystemClassLoader__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ClassLoader_isAncestorOf___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_OBJECT java_lang_ClassLoader_findResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_findResources___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_findLibrary___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getPackage___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_ClassLoader_getPackages__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ClassLoader_definePackage___java_lang_String_java_lang_String_java_lang_String_java_lang_String_java_lang_String_java_lang_String_java_lang_String_java_net_URL(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_OBJECT n5, JAVA_OBJECT n6, JAVA_OBJECT n7, JAVA_OBJECT n8);
JAVA_OBJECT java_lang_ClassLoader_getSigners___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ClassLoader_setSigners___java_lang_Class_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_ClassLoader_getStackClassLoader___int(JAVA_INT n1);
JAVA_OBJECT java_lang_ClassLoader_callerClassLoader__();
void java_lang_ClassLoader_loadLibraryWithClassLoader___java_lang_String_java_lang_ClassLoader(JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_ClassLoader_loadLibraryWithPath___java_lang_String_java_lang_ClassLoader_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_lang_ClassLoader_setClassAssertionStatus___java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_lang_ClassLoader_setPackageAssertionStatus___java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_lang_ClassLoader_setDefaultAssertionStatus___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_lang_ClassLoader_clearAssertionStatus__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ClassLoader_getClassAssertionStatus___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_ClassLoader_getPackageAssertionStatus___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_ClassLoader_getDefaultAssertionStatus__(JAVA_OBJECT me);
void java_lang_ClassLoader___CLINIT_();

#endif
