#ifndef __JAVA_LANG_REF_REFERENCEQUEUE__
#define __JAVA_LANG_REF_REFERENCEQUEUE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ref_ReferenceQueue 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_Reference
#define XMLVM_FORWARD_DECL_java_lang_ref_Reference
XMLVM_FORWARD_DECL(java_lang_ref_Reference)
#endif
// Class declarations for java.lang.ref.ReferenceQueue
XMLVM_DEFINE_CLASS(java_lang_ref_ReferenceQueue, 6, XMLVM_ITABLE_SIZE_java_lang_ref_ReferenceQueue)

extern JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue;
extern JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ref_ReferenceQueue_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_ReferenceQueue
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ref_ReferenceQueue \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT references_; \
        JAVA_INT head_; \
        JAVA_INT tail_; \
        JAVA_BOOLEAN empty_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ref_ReferenceQueue \
    } java_lang_ref_ReferenceQueue

struct java_lang_ref_ReferenceQueue {
    __TIB_DEFINITION_java_lang_ref_ReferenceQueue* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ref_ReferenceQueue;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
#define XMLVM_FORWARD_DECL_java_lang_ref_ReferenceQueue
typedef struct java_lang_ref_ReferenceQueue java_lang_ref_ReferenceQueue;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ref_ReferenceQueue 6

void __INIT_java_lang_ref_ReferenceQueue();
void __INIT_IMPL_java_lang_ref_ReferenceQueue();
void __DELETE_java_lang_ref_ReferenceQueue(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ref_ReferenceQueue(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ref_ReferenceQueue();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ref_ReferenceQueue();
JAVA_INT java_lang_ref_ReferenceQueue_GET_DEFAULT_QUEUE_SIZE();
void java_lang_ref_ReferenceQueue_PUT_DEFAULT_QUEUE_SIZE(JAVA_INT v);
void java_lang_ref_ReferenceQueue___INIT___(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ref_ReferenceQueue_newArray___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_lang_ref_ReferenceQueue_poll__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ref_ReferenceQueue_remove__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ref_ReferenceQueue_remove___long(JAVA_OBJECT me, JAVA_LONG n1);
JAVA_BOOLEAN java_lang_ref_ReferenceQueue_enqueue___java_lang_ref_Reference(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
