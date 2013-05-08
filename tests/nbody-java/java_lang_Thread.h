#ifndef __JAVA_LANG_THREAD__
#define __JAVA_LANG_THREAD__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Thread 1
// Implemented interfaces:
#include "java_lang_Runnable.h"
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
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassLoader
#define XMLVM_FORWARD_DECL_java_lang_ClassLoader
XMLVM_FORWARD_DECL(java_lang_ClassLoader)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StackTraceElement
#define XMLVM_FORWARD_DECL_java_lang_StackTraceElement
XMLVM_FORWARD_DECL(java_lang_StackTraceElement)
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
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread_State
#define XMLVM_FORWARD_DECL_java_lang_Thread_State
XMLVM_FORWARD_DECL(java_lang_Thread_State)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread_UncaughtExceptionHandler
#define XMLVM_FORWARD_DECL_java_lang_Thread_UncaughtExceptionHandler
XMLVM_FORWARD_DECL(java_lang_Thread_UncaughtExceptionHandler)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_ThreadGroup
#define XMLVM_FORWARD_DECL_java_lang_ThreadGroup
XMLVM_FORWARD_DECL(java_lang_ThreadGroup)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_HashMap
#define XMLVM_FORWARD_DECL_java_util_HashMap
XMLVM_FORWARD_DECL(java_util_HashMap)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Map
#define XMLVM_FORWARD_DECL_java_util_Map
XMLVM_FORWARD_DECL(java_util_Map)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Condition
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Condition)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_Mutex
XMLVM_FORWARD_DECL(org_xmlvm_runtime_Mutex)
#endif
// Class declarations for java.lang.Thread
XMLVM_DEFINE_CLASS(java_lang_Thread, 7, XMLVM_ITABLE_SIZE_java_lang_Thread)

extern JAVA_OBJECT __CLASS_java_lang_Thread;
extern JAVA_OBJECT __CLASS_java_lang_Thread_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Thread_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Thread_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Thread
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Thread \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT xmlvmExceptionEnv_; \
        JAVA_OBJECT xmlvmException_; \
        JAVA_OBJECT ptBuffers_; \
        JAVA_LONG threadId_; \
        JAVA_OBJECT threadName_; \
        JAVA_INT priority_; \
        JAVA_BOOLEAN daemon_; \
        JAVA_BOOLEAN alive_; \
        JAVA_OBJECT targetRunnable_; \
        JAVA_OBJECT threadGroup_; \
        JAVA_OBJECT threadState_; \
        JAVA_BOOLEAN interrupted_; \
        JAVA_OBJECT waitingCondition_; \
        JAVA_OBJECT threadLocalMap_; \
        JAVA_LONG nativeThreadId_; \
        JAVA_OBJECT action_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Thread \
    } java_lang_Thread

struct java_lang_Thread {
    __TIB_DEFINITION_java_lang_Thread* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Thread;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
typedef struct java_lang_Thread java_lang_Thread;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Thread 7
#define XMLVM_VTABLE_IDX_java_lang_Thread_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_Thread_run__ 6
#define XMLVM_VTABLE_IDX_java_lang_Thread_toString__ 5

void __INIT_java_lang_Thread();
void __INIT_IMPL_java_lang_Thread();
void __DELETE_java_lang_Thread(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Thread(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Thread();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Thread();
void xmlvm_init_native_java_lang_Thread();
JAVA_OBJECT java_lang_Thread_GET_threadSafetyMutex();
void java_lang_Thread_PUT_threadSafetyMutex(JAVA_OBJECT v);
JAVA_LONG java_lang_Thread_GET_nextThreadId();
void java_lang_Thread_PUT_nextThreadId(JAVA_LONG v);
JAVA_INT java_lang_Thread_GET_numberOfActiveNonDaemonThreads();
void java_lang_Thread_PUT_numberOfActiveNonDaemonThreads(JAVA_INT v);
JAVA_OBJECT java_lang_Thread_GET_threadMap();
void java_lang_Thread_PUT_threadMap(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_GET_mainThreadGroup();
void java_lang_Thread_PUT_mainThreadGroup(JAVA_OBJECT v);
JAVA_INT java_lang_Thread_GET_MAX_PRIORITY();
void java_lang_Thread_PUT_MAX_PRIORITY(JAVA_INT v);
JAVA_INT java_lang_Thread_GET_MIN_PRIORITY();
void java_lang_Thread_PUT_MIN_PRIORITY(JAVA_INT v);
JAVA_INT java_lang_Thread_GET_NORM_PRIORITY();
void java_lang_Thread_PUT_NORM_PRIORITY(JAVA_INT v);
JAVA_LONG java_lang_Thread_getNextThreadId__();
void java_lang_Thread_addSelfToMap__(JAVA_OBJECT me);
void java_lang_Thread_removeSelfFromMap__(JAVA_OBJECT me);
JAVA_LONG java_lang_Thread_getNativeThreadId__(JAVA_OBJECT me);
void java_lang_Thread___INIT____long(JAVA_OBJECT me, JAVA_LONG n1);
void java_lang_Thread_initMainThread__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getThread___long(JAVA_LONG n1);
// Vtable index: 1
JAVA_BOOLEAN java_lang_Thread_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread_initThread___java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_lang_Thread___INIT___(JAVA_OBJECT me);
void java_lang_Thread___INIT____java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread___INIT____java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Thread___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_LONG n4);
void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_Runnable_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_lang_Thread___INIT____java_lang_ThreadGroup_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Thread_setInterruptAction___java_lang_Runnable(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_INT java_lang_Thread_activeCount__();
void java_lang_Thread_checkAccess__(JAVA_OBJECT me);
JAVA_INT java_lang_Thread_countStackFrames__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_currentThread__();
JAVA_LONG java_lang_Thread_getCurrentThreadNativeId__();
void java_lang_Thread_destroy__(JAVA_OBJECT me);
void java_lang_Thread_dumpStack__();
JAVA_INT java_lang_Thread_enumerate___java_lang_Thread_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Thread_getAllStackTraces__();
JAVA_OBJECT java_lang_Thread_getContextClassLoader__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getDefaultUncaughtExceptionHandler__();
JAVA_LONG java_lang_Thread_getId__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getName__(JAVA_OBJECT me);
JAVA_INT java_lang_Thread_getPriority__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getStackTrace__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getState__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getThreadGroup__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Thread_getThreadLocal___java_lang_ThreadLocal(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Thread_getUncaughtExceptionHandler__(JAVA_OBJECT me);
void java_lang_Thread_interrupt__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Thread_interrupted__();
JAVA_BOOLEAN java_lang_Thread_isAlive__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Thread_isDaemon__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_Thread_isInterrupted__(JAVA_OBJECT me);
void java_lang_Thread_join__(JAVA_OBJECT me);
void java_lang_Thread_join___long(JAVA_OBJECT me, JAVA_LONG n1);
void java_lang_Thread_join___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2);
void java_lang_Thread_resume__(JAVA_OBJECT me);
void java_lang_Thread_run0___long(JAVA_OBJECT me, JAVA_LONG n1);
JAVA_BOOLEAN java_lang_Thread_stackTracesEnabled__();
void java_lang_Thread_threadTerminating__(JAVA_OBJECT me);
// Vtable index: 6
void java_lang_Thread_run__(JAVA_OBJECT me);
void java_lang_Thread_setContextClassLoader___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread_setDaemon___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_lang_Thread_setDefaultUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT n1);
void java_lang_Thread_setName___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread_setPriority___int(JAVA_OBJECT me, JAVA_INT n1);
void java_lang_Thread_setThreadLocal___java_lang_ThreadLocal_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread_sleep___long(JAVA_LONG n1);
void java_lang_Thread_sleep___long_int(JAVA_LONG n1, JAVA_INT n2);
void java_lang_Thread_start__(JAVA_OBJECT me);
void java_lang_Thread_start0__(JAVA_OBJECT me);
void java_lang_Thread_stop__(JAVA_OBJECT me);
void java_lang_Thread_stop___java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread_suspend__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_Thread_toString__(JAVA_OBJECT me);
void java_lang_Thread_yield__();
JAVA_BOOLEAN java_lang_Thread_holdsLock___java_lang_Object(JAVA_OBJECT n1);
void java_lang_Thread_setWaitingCondition___org_xmlvm_runtime_Condition(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Thread___CLINIT_();

#endif
