#ifndef __ORG_XMLVM_RUNTIME_XMLVMUTIL__
#define __ORG_XMLVM_RUNTIME_XMLVMUTIL__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMUtil 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
XMLVM_FORWARD_DECL(java_io_PrintStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMOutputStream
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMOutputStream
XMLVM_FORWARD_DECL(org_xmlvm_runtime_XMLVMOutputStream)
#endif
// Class declarations for org.xmlvm.runtime.XMLVMUtil
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_XMLVMUtil, 6, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMUtil)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMUtil;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMUtil_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMUtil_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMUtil_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMUtil
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMUtil \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMUtil \
    } org_xmlvm_runtime_XMLVMUtil

struct org_xmlvm_runtime_XMLVMUtil {
    __TIB_DEFINITION_org_xmlvm_runtime_XMLVMUtil* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMUtil;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
typedef struct org_xmlvm_runtime_XMLVMUtil org_xmlvm_runtime_XMLVMUtil;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_XMLVMUtil 6

void __INIT_org_xmlvm_runtime_XMLVMUtil();
void __INIT_IMPL_org_xmlvm_runtime_XMLVMUtil();
void __DELETE_org_xmlvm_runtime_XMLVMUtil(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMUtil(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMUtil();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMUtil();
void org_xmlvm_runtime_XMLVMUtil___INIT___(JAVA_OBJECT me);
void org_xmlvm_runtime_XMLVMUtil_init__();
void org_xmlvm_runtime_XMLVMUtil_notImplemented__();
JAVA_OBJECT org_xmlvm_runtime_XMLVMUtil_getCurrentWorkingDirectory__();

#endif
