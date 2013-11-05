#ifndef __ORG_XMLVM_RUNTIME_REDTYPEMARKER__
#define __ORG_XMLVM_RUNTIME_REDTYPEMARKER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_RedTypeMarker 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for org.xmlvm.runtime.RedTypeMarker
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_RedTypeMarker, 6, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_RedTypeMarker)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_RedTypeMarker
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_RedTypeMarker \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_RedTypeMarker \
    } org_xmlvm_runtime_RedTypeMarker

struct org_xmlvm_runtime_RedTypeMarker {
    __TIB_DEFINITION_org_xmlvm_runtime_RedTypeMarker* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_RedTypeMarker;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_RedTypeMarker
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_RedTypeMarker
typedef struct org_xmlvm_runtime_RedTypeMarker org_xmlvm_runtime_RedTypeMarker;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_RedTypeMarker 6

void __INIT_org_xmlvm_runtime_RedTypeMarker();
void __INIT_IMPL_org_xmlvm_runtime_RedTypeMarker();
void __DELETE_org_xmlvm_runtime_RedTypeMarker(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_RedTypeMarker(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_RedTypeMarker();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_RedTypeMarker();
void org_xmlvm_runtime_RedTypeMarker___INIT___(JAVA_OBJECT me);

#endif
