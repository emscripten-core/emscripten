#ifndef __JAVA_LANG_THREADGROUP__
#define __JAVA_LANG_THREADGROUP__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ThreadGroup 1
// Implemented interfaces:
#include "java_lang_Thread_UncaughtExceptionHandler.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
XMLVM_FORWARD_DECL(java_io_PrintStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_SecurityManager
#define XMLVM_FORWARD_DECL_java_lang_SecurityManager
XMLVM_FORWARD_DECL(java_lang_SecurityManager)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
XMLVM_FORWARD_DECL(java_lang_Thread)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenGroupsLock
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenGroupsLock
XMLVM_FORWARD_DECL(java_lang_ThreadGroup_ChildrenGroupsLock)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenThreadsLock
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup_ChildrenThreadsLock
XMLVM_FORWARD_DECL(java_lang_ThreadGroup_ChildrenThreadsLock)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
// Class declarations for java.lang.ThreadGroup
XMLVM_DEFINE_CLASS(java_lang_ThreadGroup, 7, XMLVM_ITABLE_SIZE_java_lang_ThreadGroup)

extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ThreadGroup_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ThreadGroup \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT name_; \
        JAVA_INT maxPriority_; \
        JAVA_OBJECT parent_; \
        JAVA_INT numThreads_; \
        JAVA_OBJECT childrenThreads_; \
        JAVA_INT numGroups_; \
        JAVA_OBJECT childrenGroups_; \
        JAVA_OBJECT childrenGroupsLock_; \
        JAVA_OBJECT childrenThreadsLock_; \
        JAVA_BOOLEAN isDaemon_; \
        JAVA_BOOLEAN isDestroyed_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ThreadGroup \
    } java_lang_ThreadGroup

struct java_lang_ThreadGroup {
    __TIB_DEFINITION_java_lang_ThreadGroup* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ThreadGroup;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup
typedef struct java_lang_ThreadGroup java_lang_ThreadGroup;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ThreadGroup 7
#define XMLVM_VTABLE_IDX_java_lang_ThreadGroup_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable 6

void __INIT_java_lang_ThreadGroup();
void __INIT_IMPL_java_lang_ThreadGroup();
void __DELETE_java_lang_ThreadGroup(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ThreadGroup(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ThreadGroup();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ThreadGroup();
void java_lang_ThreadGroup___INIT___(JAVA_OBJECT me);
void java_lang_ThreadGroup___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup___INIT____java_lang_ThreadGroup_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_ThreadGroup___INIT____java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_ThreadGroup_activeCount__(JAVA_OBJECT me);
JAVA_INT java_lang_ThreadGroup_activeGroupCount__(JAVA_OBJECT me);
void java_lang_ThreadGroup_add___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_add___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_ThreadGroup_allowThreadSuspension___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_lang_ThreadGroup_checkAccess__(JAVA_OBJECT me);
void java_lang_ThreadGroup_destroy__(JAVA_OBJECT me);
void java_lang_ThreadGroup_destroyIfEmptyDaemon__(JAVA_OBJECT me);
JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_Thread_1ARRAY_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_ThreadGroup_enumerate___java_lang_ThreadGroup_1ARRAY_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
JAVA_INT java_lang_ThreadGroup_enumerateGeneric___java_lang_Object_1ARRAY_boolean_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_INT n3, JAVA_BOOLEAN n4);
JAVA_INT java_lang_ThreadGroup_getMaxPriority__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ThreadGroup_getName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_ThreadGroup_getParent__(JAVA_OBJECT me);
void java_lang_ThreadGroup_interrupt__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ThreadGroup_isDaemon__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ThreadGroup_isDestroyed__(JAVA_OBJECT me);
void java_lang_ThreadGroup_list__(JAVA_OBJECT me);
void java_lang_ThreadGroup_list___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_BOOLEAN java_lang_ThreadGroup_parentOf___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_remove___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_remove___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_resume__(JAVA_OBJECT me);
void java_lang_ThreadGroup_setDaemon___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_lang_ThreadGroup_setMaxPriority___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_ThreadGroup_setParent___java_lang_ThreadGroup(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ThreadGroup_stop__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ThreadGroup_stopHelper__(JAVA_OBJECT me);
void java_lang_ThreadGroup_suspend__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_ThreadGroup_suspendHelper__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_ThreadGroup_toString__(JAVA_OBJECT me);
// Vtable index: 6
void java_lang_ThreadGroup_uncaughtException___java_lang_Thread_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);

#endif
