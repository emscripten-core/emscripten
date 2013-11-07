
#include "xmlvm.h"
#include "org_xmlvm_runtime_Mutex.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <pthread.h>

void nativeCleanup(void* me, void* client_data)
{
    JAVA_OBJECT m = ((org_xmlvm_runtime_Mutex*) me)->fields.org_xmlvm_runtime_Mutex.nativeMutex_;
    org_xmlvm_runtime_Mutex_addNativeMutexToFinalizerQueue___java_lang_Object(m);
}
//XMLVM_END_NATIVE_IMPLEMENTATION

void org_xmlvm_runtime_Mutex_destroyNativeMutex___java_lang_Object(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Mutex_destroyNativeMutex___java_lang_Object]
    pthread_mutex_destroy(n1);
    free(n1);
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Mutex_initNativeInstance__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Mutex_initNativeInstance__]
    pthread_mutex_t* m = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(m, NULL);
    ((org_xmlvm_runtime_Mutex*) me)->fields.org_xmlvm_runtime_Mutex.nativeMutex_ = m;
    // Register a finalizer for native cleanup
    XMLVM_FINALIZE(me, nativeCleanup);
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Mutex_lock__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Mutex_lock__]
    JAVA_OBJECT m = ((org_xmlvm_runtime_Mutex*) me)->fields.org_xmlvm_runtime_Mutex.nativeMutex_;
    int result = pthread_mutex_lock(m);
    if (result != 0) {
        printf("Error locking mutex! %i\n", result);
    }
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Mutex_unlock__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Mutex_unlock__]
    JAVA_OBJECT m = ((org_xmlvm_runtime_Mutex*) me)->fields.org_xmlvm_runtime_Mutex.nativeMutex_;
    int result = pthread_mutex_unlock(m);
    if (result != 0) {
        printf("Error unlocking mutex! %i\n", result);
    }
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_xmlvm_runtime_Mutex()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Mutex_lock__
    __TIB_org_xmlvm_runtime_Mutex.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Mutex_lock__] = 
        (VTABLE_PTR) org_xmlvm_runtime_Mutex_lock__;
#endif
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Mutex_unlock__
    __TIB_org_xmlvm_runtime_Mutex.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Mutex_unlock__] = 
        (VTABLE_PTR) org_xmlvm_runtime_Mutex_unlock__;
#endif
}
