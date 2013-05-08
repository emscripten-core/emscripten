#ifndef __ORG_XMLVM_RUNTIME_XMLVMOUTPUTSTREAM__
#define __ORG_XMLVM_RUNTIME_XMLVMOUTPUTSTREAM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMOutputStream 2
// Implemented interfaces:
// Super Class:
#include "java_io_OutputStream.h"

// Circular references:
// Class declarations for org.xmlvm.runtime.XMLVMOutputStream
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_XMLVMOutputStream, 12, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMOutputStream)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMOutputStream_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMOutputStream
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMOutputStream \
    __INSTANCE_FIELDS_java_io_OutputStream; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMOutputStream \
    } org_xmlvm_runtime_XMLVMOutputStream

struct org_xmlvm_runtime_XMLVMOutputStream {
    __TIB_DEFINITION_org_xmlvm_runtime_XMLVMOutputStream* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMOutputStream;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMOutputStream
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMOutputStream
typedef struct org_xmlvm_runtime_XMLVMOutputStream org_xmlvm_runtime_XMLVMOutputStream;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_XMLVMOutputStream 12
#define XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMOutputStream_write___int 11

void __INIT_org_xmlvm_runtime_XMLVMOutputStream();
void __INIT_IMPL_org_xmlvm_runtime_XMLVMOutputStream();
void __DELETE_org_xmlvm_runtime_XMLVMOutputStream(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMOutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMOutputStream();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMOutputStream();
void org_xmlvm_runtime_XMLVMOutputStream___INIT___(JAVA_OBJECT me);
// Vtable index: 11
void org_xmlvm_runtime_XMLVMOutputStream_write___int(JAVA_OBJECT me, JAVA_INT n1);
void org_xmlvm_runtime_XMLVMOutputStream_nativeWrite___int(JAVA_INT n1);

#endif
