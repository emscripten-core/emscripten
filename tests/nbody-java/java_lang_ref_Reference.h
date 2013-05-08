#ifndef __JAVA_LANG_REF_REFERENCE__
#define __JAVA_LANG_REF_REFERENCE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ref_Reference 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
#define XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
XMLVM_FORWARD_DECL(java_lang_ref_ReferenceQueue)
#endif
// Class declarations for java.lang.ref.Reference
XMLVM_DEFINE_CLASS(java_lang_ref_Reference, 6, XMLVM_ITABLE_SIZE_java_lang_ref_Reference)

extern JAVA_OBJECT __CLASS_java_lang_ref_Reference;
extern JAVA_OBJECT __CLASS_java_lang_ref_Reference_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_Reference_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_Reference_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_Reference
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ref_Reference \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT obj_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_Reference \
    } java_lang_ref_Reference

struct java_lang_ref_Reference {
    __TIB_DEFINITION_java_lang_ref_Reference* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ref_Reference;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_Reference
#define XMLVM_FORWARD_DECL_java_lang_ref_Reference
typedef struct java_lang_ref_Reference java_lang_ref_Reference;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ref_Reference 6

void __INIT_java_lang_ref_Reference();
void __INIT_IMPL_java_lang_ref_Reference();
void __DELETE_java_lang_ref_Reference(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ref_Reference(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ref_Reference();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_Reference();
void java_lang_ref_Reference___INIT___(JAVA_OBJECT me);
void java_lang_ref_Reference_clear__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ref_Reference_enqueue__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ref_Reference_get__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ref_Reference_isEnqueued__(JAVA_OBJECT me);
void java_lang_ref_Reference_initReference___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ref_Reference_initReference___java_lang_Object_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_BOOLEAN java_lang_ref_Reference_enqueueImpl__(JAVA_OBJECT me);
void java_lang_ref_Reference_dequeue__(JAVA_OBJECT me);

#endif
