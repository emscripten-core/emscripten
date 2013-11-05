#ifndef __JAVA_LANG_REF_WEAKREFERENCE__
#define __JAVA_LANG_REF_WEAKREFERENCE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ref_WeakReference 0
// Implemented interfaces:
// Super Class:
#include "java_lang_ref_Reference.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
#define XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
XMLVM_FORWARD_DECL(java_lang_ref_ReferenceQueue)
#endif
// Class declarations for java.lang.ref.WeakReference
XMLVM_DEFINE_CLASS(java_lang_ref_WeakReference, 6, XMLVM_ITABLE_SIZE_java_lang_ref_WeakReference)

extern JAVA_OBJECT __CLASS_java_lang_ref_WeakReference;
extern JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_WeakReference_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_WeakReference
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ref_WeakReference \
    __INSTANCE_FIELDS_java_lang_ref_Reference; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_WeakReference \
    } java_lang_ref_WeakReference

struct java_lang_ref_WeakReference {
    __TIB_DEFINITION_java_lang_ref_WeakReference* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ref_WeakReference;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_WeakReference
#define XMLVM_FORWARD_DECL_java_lang_ref_WeakReference
typedef struct java_lang_ref_WeakReference java_lang_ref_WeakReference;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ref_WeakReference 6

void __INIT_java_lang_ref_WeakReference();
void __INIT_IMPL_java_lang_ref_WeakReference();
void __DELETE_java_lang_ref_WeakReference(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ref_WeakReference(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ref_WeakReference();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_WeakReference();
void java_lang_ref_WeakReference___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ref_WeakReference___INIT____java_lang_Object_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);

#endif
