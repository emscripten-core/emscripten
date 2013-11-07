#ifndef __ORG_XMLVM_RUNTIME_MUTEX__
#define __ORG_XMLVM_RUNTIME_MUTEX__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_org_xmlvm_runtime_Mutex 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_util_LinkedList
#define XMLVM_FORWARD_DECL_java_util_LinkedList
XMLVM_FORWARD_DECL(java_util_LinkedList)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Queue
#define XMLVM_FORWARD_DECL_java_util_Queue
XMLVM_FORWARD_DECL(java_util_Queue)
#endif
// Class declarations for org.xmlvm.runtime.Mutex
XMLVM_DEFINE_CLASS(org_xmlvm_runtime_Mutex, 6, XMLVM_ITABLE_SIZE_org_xmlvm_runtime_Mutex)

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_1ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_2ARRAY;
extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_Mutex_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_Mutex
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_org_xmlvm_runtime_Mutex \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT nativeMutex_; \
        __ADDITIONAL_INSTANCE_FIELDS_org_xmlvm_runtime_Mutex \
    } org_xmlvm_runtime_Mutex

struct org_xmlvm_runtime_Mutex {
    __TIB_DEFINITION_org_xmlvm_runtime_Mutex* tib;
    struct {
        __INSTANCE_FIELDS_org_xmlvm_runtime_Mutex;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
typedef struct org_xmlvm_runtime_Mutex org_xmlvm_runtime_Mutex;
#endif

#define XMLVM_VTABLE_SIZE_org_xmlvm_runtime_Mutex 6

void __INIT_org_xmlvm_runtime_Mutex();
void __INIT_IMPL_org_xmlvm_runtime_Mutex();
void __DELETE_org_xmlvm_runtime_Mutex(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_Mutex(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_org_xmlvm_runtime_Mutex();
JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_Mutex();
void xmlvm_init_native_org_xmlvm_runtime_Mutex();
JAVA_OBJECT org_xmlvm_runtime_Mutex_GET_finalizableNativeMutexList();
void org_xmlvm_runtime_Mutex_PUT_finalizableNativeMutexList(JAVA_OBJECT v);
void org_xmlvm_runtime_Mutex_addNativeMutexToFinalizerQueue___java_lang_Object(JAVA_OBJECT n1);
void org_xmlvm_runtime_Mutex_destroyFinalizableNativeMutexes__();
void org_xmlvm_runtime_Mutex_destroyNativeMutex___java_lang_Object(JAVA_OBJECT n1);
void org_xmlvm_runtime_Mutex___INIT___(JAVA_OBJECT me);
void org_xmlvm_runtime_Mutex_initNativeInstance__(JAVA_OBJECT me);
void org_xmlvm_runtime_Mutex_lock__(JAVA_OBJECT me);
void org_xmlvm_runtime_Mutex_unlock__(JAVA_OBJECT me);
void org_xmlvm_runtime_Mutex___CLINIT_();

#endif
