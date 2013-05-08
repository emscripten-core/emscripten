
#include "xmlvm.h"
#include "org_xmlvm_runtime_FinalizerNotifier.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

void org_xmlvm_runtime_FinalizerNotifier_preventGarbageCollection___boolean(JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_FinalizerNotifier_preventGarbageCollection___boolean]
#ifndef XMLVM_NO_GC
    if (n1) {
        // Disable garbage collection.  Even GC_gcollect calls will be ineffective.
        GC_disable();
    } else {
        // Reenable garbage collection.  GC_disable() and GC_enable() calls
        // nest.  Garbage collection is enabled if the number of calls to both
        // functions is equal.
        GC_enable();
    }
#endif
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_currentThreadIsFinalizerThread___java_lang_Thread(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_FinalizerNotifier_currentThreadIsFinalizerThread___java_lang_Thread]

    // Determine if the current thread is the finalizer thread (the 1st
    // parameter).

    // This method is native since any methods added to proxy classes are not
    // visible to this class, such as Thread.getCurrentThreadNativeId().

    // return Thread.getCurrentThreadNativeId() == finalizerThread.getNativeThreadId();
    return java_lang_Thread_getCurrentThreadNativeId__() == java_lang_Thread_getNativeThreadId__(n1);

    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN org_xmlvm_runtime_FinalizerNotifier_shouldInvokeFinalizers__()
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_FinalizerNotifier_shouldInvokeFinalizers__]
#ifndef XMLVM_NO_GC
    return GC_should_invoke_finalizers();
#else
    return 0;
#endif
    //XMLVM_END_NATIVE
}

JAVA_INT org_xmlvm_runtime_FinalizerNotifier_invokeFinalizers__()
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_FinalizerNotifier_invokeFinalizers__]
#ifndef XMLVM_NO_GC
    return GC_invoke_finalizers();
#else
    return 0;
#endif
    //XMLVM_END_NATIVE
}

