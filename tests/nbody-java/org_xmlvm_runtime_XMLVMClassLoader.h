#ifndef __ORG_XMLVM_RUNTIME_XMLVMCLASSLOADER__
#define __ORG_XMLVM_RUNTIME_XMLVMCLASSLOADER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMClassLoader 0
// Implemented interfaces:
// Super Class:
#include "java_lang_ClassLoader.h"

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
// Class declarations for org.xmlvm.runtime.XMLVMClassLoader
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_XMLVMClassLoader, 8, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMClassLoader)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMClassLoader_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMClassLoader
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMClassLoader \
    __INSTANCE_FIELDS_java_lang_ClassLoader; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMClassLoader \
    } org_xmlvm_runtime_XMLVMClassLoader

struct org_xmlvm_runtime_XMLVMClassLoader {
    __TIB_DEFINITION_org_xmlvm_runtime_XMLVMClassLoader* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMClassLoader;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMClassLoader
typedef struct org_xmlvm_runtime_XMLVMClassLoader org_xmlvm_runtime_XMLVMClassLoader;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_XMLVMClassLoader 8
#define XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMClassLoader_findClass___java_lang_String 6
#define XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMClassLoader_findResource___java_lang_String 7

void __INIT_org_xmlvm_runtime_XMLVMClassLoader();
void __INIT_IMPL_org_xmlvm_runtime_XMLVMClassLoader();
void __DELETE_org_xmlvm_runtime_XMLVMClassLoader(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMClassLoader(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMClassLoader();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMClassLoader();
void org_xmlvm_runtime_XMLVMClassLoader___INIT___(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT org_xmlvm_runtime_XMLVMClassLoader_findClass___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
JAVA_OBJECT org_xmlvm_runtime_XMLVMClassLoader_findResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
