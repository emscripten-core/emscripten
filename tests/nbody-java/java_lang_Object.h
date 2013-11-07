#ifndef __JAVA_LANG_OBJECT__
#define __JAVA_LANG_OBJECT__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Object 0
// Implemented interfaces:
// Super Class:

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Math
#define XMLVM_FORWARD_DECL_java_lang_Math
XMLVM_FORWARD_DECL(java_lang_Math)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object_AddedMembers
#define XMLVM_FORWARD_DECL_java_lang_Object_AddedMembers
XMLVM_FORWARD_DECL(java_lang_Object_AddedMembers)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
XMLVM_FORWARD_DECL(java_lang_Thread)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_List
#define XMLVM_FORWARD_DECL_java_util_List
XMLVM_FORWARD_DECL(java_util_List)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Condition)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Mutex)
#endif
// Class declarations for java.lang.Object
XMLVM_DEFINE_CLASS(java_lang_Object, 6, XMLVM_ITABLE_SIZE_java_lang_Object)

extern JAVA_OBJECT __CLASS_java_lang_Object;
extern JAVA_OBJECT __CLASS_java_lang_Object_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Object_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Object_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Object
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Object \
    struct { \
        JAVA_OBJECT addedMembers_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Object \
    } java_lang_Object

struct java_lang_Object {
    __TIB_DEFINITION_java_lang_Object* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Object;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
typedef struct java_lang_Object java_lang_Object;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Object 6
#define XMLVM_VTABLE_IDX_java_lang_Object_clone__ 0
#define XMLVM_VTABLE_IDX_java_lang_Object_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Object_finalize_java_lang_Object__ 2
#define XMLVM_VTABLE_IDX_java_lang_Object_getClass__ 3
#define XMLVM_VTABLE_IDX_java_lang_Object_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_Object_toString__ 5

void __INIT_java_lang_Object();
void __INIT_IMPL_java_lang_Object();
void __DELETE_java_lang_Object(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Object(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Object();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Object();
void xmlvm_init_native_java_lang_Object();
JAVA_OBJECT java_lang_Object_GET_staticMutex();
void java_lang_Object_PUT_staticMutex(JAVA_OBJECT v);
void java_lang_Object_initNativeLayer__();
void java_lang_Object___INIT___(JAVA_OBJECT me);
// Vtable index: 0
JAVA_OBJECT java_lang_Object_clone__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Object_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 2
void java_lang_Object_finalize_java_lang_Object__(JAVA_OBJECT me);
// Vtable index: 3
JAVA_OBJECT java_lang_Object_getClass__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_Object_hashCode__(JAVA_OBJECT me);
void java_lang_Object_notify__(JAVA_OBJECT me);
void java_lang_Object_notifyAll__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Object_toString__(JAVA_OBJECT me);
void java_lang_Object_wait__(JAVA_OBJECT me);
void java_lang_Object_wait___long(JAVA_OBJECT me, JAVA_LONG n1);
void java_lang_Object_wait___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2);
void java_lang_Object_establishLock___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Object_prepareForUnlock__(JAVA_OBJECT me);
void java_lang_Object_syncLock___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Object_syncUnlock__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Object_acquireLockRecursive__(JAVA_OBJECT me);
void java_lang_Object_releaseLockRecursive__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Object_enqueueNewCondition__(JAVA_OBJECT me);
void java_lang_Object_checkSynchronized___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Object_removeThreadNotification___org_xmlvm_runtime_Condition(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Object_getRandInclusive___int_int(JAVA_INT n1, JAVA_INT n2);
JAVA_INT java_lang_Object_preWait___java_lang_Thread_org_xmlvm_runtime_Condition(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_BOOLEAN java_lang_Object_postWait___java_lang_Thread_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_Object_wait2__(JAVA_OBJECT me);
void java_lang_Object_wait2___long(JAVA_OBJECT me, JAVA_LONG n1);
void java_lang_Object_notify2__(JAVA_OBJECT me);
void java_lang_Object_notifyAll2__(JAVA_OBJECT me);
void java_lang_Object_interruptWait___org_xmlvm_runtime_Condition(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Object___CLINIT_();

#endif
