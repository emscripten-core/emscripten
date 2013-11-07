#ifndef __ORG_XMLVM_RUNTIME_XMLVMARRAY__
#define __ORG_XMLVM_RUNTIME_XMLVMARRAY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMArray 0
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Cloneable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
// Class declarations for org.xmlvm.runtime.XMLVMArray
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_XMLVMArray, 6, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_XMLVMArray)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMArray
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMArray \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT type_; \
        JAVA_INT length_; \
        JAVA_OBJECT array_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMArray \
    } org_xmlvm_runtime_XMLVMArray

struct org_xmlvm_runtime_XMLVMArray {
    __TIB_DEFINITION_org_xmlvm_runtime_XMLVMArray* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_XMLVMArray;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMArray
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMArray
typedef struct org_xmlvm_runtime_XMLVMArray org_xmlvm_runtime_XMLVMArray;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_XMLVMArray 6
#define XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMArray_clone__ 0

void __INIT_org_xmlvm_runtime_XMLVMArray();
void __INIT_IMPL_org_xmlvm_runtime_XMLVMArray();
void __DELETE_org_xmlvm_runtime_XMLVMArray(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMArray(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMArray();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMArray();
void xmlvm_init_native_org_xmlvm_runtime_XMLVMArray();
JAVA_LONG org_xmlvm_runtime_XMLVMArray_GET_serialVersionUID();
void org_xmlvm_runtime_XMLVMArray_PUT_serialVersionUID(JAVA_LONG v);
void org_xmlvm_runtime_XMLVMArray_initNativeLayer__();
void org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3);
JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2);
JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3);
JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray(JAVA_OBJECT n1, JAVA_OBJECT n2);
void org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 0
JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_clone__(JAVA_OBJECT me);
void org_xmlvm_runtime_XMLVMArray___CLINIT_();

#endif
