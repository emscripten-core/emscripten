
#include "xmlvm.h"
#include "java_lang_Thread.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <pthread.h>
#ifdef __OBJC__
#import <Foundation/NSAutoreleasePool.h>
#endif


void threadRunner(JAVA_OBJECT me)
{
    java_lang_Thread* thiz = me;
    XMLVM_JMP_BUF xmlvm_exception_env;
    JAVA_LONG nativeThreadId = (JAVA_LONG) pthread_self();
    
#ifdef __OBJC__
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
#endif
#ifdef XMLVM_ENABLE_STACK_TRACES
    createStackForNewThread(nativeThreadId);
#endif

    if (XMLVM_SETJMP(xmlvm_exception_env)) {
        // Technically, XMLVM_UNWIND_EXCEPTION() should be called, but
        // exceptions will not be used anymore and XMLVM_ENTER_METHOD() wasn't
        // called (excessive), so a compilation error would occur

        xmlvm_unhandled_exception();
    } else {
        thiz->fields.java_lang_Thread.xmlvmExceptionEnv_ = &xmlvm_exception_env;
        java_lang_Thread_run0___long(thiz, nativeThreadId);
    }

#ifdef XMLVM_ENABLE_STACK_TRACES
    destroyStackForExitingThread(nativeThreadId);
#endif
#ifdef __OBJC__
    [pool release];
#endif
}
//XMLVM_END_NATIVE_IMPLEMENTATION

void java_lang_Thread_initMainThread__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_initMainThread__]
    ((java_lang_Thread*)me)->fields.java_lang_Thread.xmlvmExceptionEnv_ = xmlvm_exception_env_main_thread;
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_Thread_activeCount__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_activeCount__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_checkAccess__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_checkAccess__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_Thread_countStackFrames__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_countStackFrames__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_Thread_getCurrentThreadNativeId__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getCurrentThreadNativeId__]
    JAVA_LONG nativeThreadId = (JAVA_LONG)pthread_self();
    return nativeThreadId;
    //XMLVM_END_NATIVE
}

void java_lang_Thread_destroy__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_destroy__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_dumpStack__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_dumpStack__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_Thread_enumerate___java_lang_Thread_1ARRAY(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_enumerate___java_lang_Thread_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_Thread_getAllStackTraces__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getAllStackTraces__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_Thread_getContextClassLoader__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getContextClassLoader__]
    XMLVMElem _r0;
    _r0.o = __NEW_org_xmlvm_runtime_XMLVMClassLoader();
    org_xmlvm_runtime_XMLVMClassLoader___INIT___(_r0.o);
    return _r0.o;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_Thread_getDefaultUncaughtExceptionHandler__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getDefaultUncaughtExceptionHandler__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_Thread_getStackTrace__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getStackTrace__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_Thread_getUncaughtExceptionHandler__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_getUncaughtExceptionHandler__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_join___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_join___long_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_resume__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_resume__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_Thread_stackTracesEnabled__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_stackTracesEnabled__]
#ifdef XMLVM_ENABLE_STACK_TRACES
    return 1;
#else
    return 0;
#endif
    //XMLVM_END_NATIVE
}

void java_lang_Thread_setContextClassLoader___java_lang_ClassLoader(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_setContextClassLoader___java_lang_ClassLoader]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_setDefaultUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_setDefaultUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_setPriority___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_setPriority___int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_start0__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_start0__]
    pthread_t pt;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&pt, &attr, threadRunner, (void *)me);
    if (rc != 0) {
        printf("ERROR creating thread. Return code: %i", rc);
        exit(-1);
    }
    pthread_attr_destroy(&attr);
    //XMLVM_END_NATIVE
}

void java_lang_Thread_stop__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_stop__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_stop___java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_stop___java_lang_Throwable]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_suspend__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_suspend__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void java_lang_Thread_yield__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_yield__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_lang_Thread_holdsLock___java_lang_Object(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Thread_holdsLock___java_lang_Object]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_java_lang_Thread()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_checkAccess__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_checkAccess__] = 
        (VTABLE_PTR) java_lang_Thread_checkAccess__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_countStackFrames__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_countStackFrames__] = 
        (VTABLE_PTR) java_lang_Thread_countStackFrames__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_destroy__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_destroy__] = 
        (VTABLE_PTR) java_lang_Thread_destroy__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_getContextClassLoader__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_getContextClassLoader__] = 
        (VTABLE_PTR) java_lang_Thread_getContextClassLoader__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_getStackTrace__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_getStackTrace__] = 
        (VTABLE_PTR) java_lang_Thread_getStackTrace__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_getUncaughtExceptionHandler__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_getUncaughtExceptionHandler__] = 
        (VTABLE_PTR) java_lang_Thread_getUncaughtExceptionHandler__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_join___long_int
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_join___long_int] = 
        (VTABLE_PTR) java_lang_Thread_join___long_int;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_resume__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_resume__] = 
        (VTABLE_PTR) java_lang_Thread_resume__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_setContextClassLoader___java_lang_ClassLoader
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_setContextClassLoader___java_lang_ClassLoader] = 
        (VTABLE_PTR) java_lang_Thread_setContextClassLoader___java_lang_ClassLoader;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_setPriority___int
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_setPriority___int] = 
        (VTABLE_PTR) java_lang_Thread_setPriority___int;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler] = 
        (VTABLE_PTR) java_lang_Thread_setUncaughtExceptionHandler___java_lang_Thread_UncaughtExceptionHandler;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_start0__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_start0__] = 
        (VTABLE_PTR) java_lang_Thread_start0__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_stop__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_stop__] = 
        (VTABLE_PTR) java_lang_Thread_stop__;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_stop___java_lang_Throwable
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_stop___java_lang_Throwable] = 
        (VTABLE_PTR) java_lang_Thread_stop___java_lang_Throwable;
#endif
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_suspend__
    __TIB_java_lang_Thread.vtable[XMLVM_VTABLE_IDX_java_lang_Thread_suspend__] = 
        (VTABLE_PTR) java_lang_Thread_suspend__;
#endif
}
