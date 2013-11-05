
#include "xmlvm.h"
#include "org_xmlvm_runtime_Condition.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION

#include "java_lang_Object_AddedMembers.h"
#include "org_xmlvm_runtime_Mutex.h"
#include <errno.h>
#include <sys/time.h>

pthread_cond_t* getConditionPtr(org_xmlvm_runtime_Condition* me)
{
    return (pthread_cond_t*)(me->fields.org_xmlvm_runtime_Condition.pthreadCondT_);
}

void nativeConditionCleanup(void* me, void* client_data)
{
    pthread_cond_t* condPtr = getConditionPtr((org_xmlvm_runtime_Condition*)me);
    pthread_cond_destroy(condPtr);
    free(condPtr);
}

//XMLVM_END_NATIVE_IMPLEMENTATION

void org_xmlvm_runtime_Condition_initNativeConditionInstance__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Condition_initNativeConditionInstance__]
    ((org_xmlvm_runtime_Condition*)me)->fields.org_xmlvm_runtime_Condition.pthreadCondT_ = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(getConditionPtr(me), NULL);

    // Register a finalizer for native cleanup
    XMLVM_FINALIZE(me, nativeConditionCleanup);
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex]
    pthread_cond_t* condPtr = getConditionPtr(me);
    JAVA_OBJECT mutex = ((org_xmlvm_runtime_Mutex*)n1)->fields.org_xmlvm_runtime_Mutex.nativeMutex_;
    int result = pthread_cond_wait(condPtr, mutex);
    if (result != 0) {
        printf("ERROR: pthread_cond_wait response was %i\n", result);
    }
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long]
    pthread_cond_t* condPtr = getConditionPtr(me);
    JAVA_OBJECT mutex = ((org_xmlvm_runtime_Mutex*)n1)->fields.org_xmlvm_runtime_Mutex.nativeMutex_;

    struct timeval now;
    gettimeofday(&now, NULL);

    struct timespec to;
    // To avoid an overflow error, we drop precision on microseconds (usec) in favor of milliseconds.
    // The loss of precision at the microsecond level is not of concern.
    JAVA_LONG msec = (now.tv_usec / 1000) + n2;
    to.tv_sec = now.tv_sec + (msec / 1000);
    to.tv_nsec = (msec % 1000) * 1000000;

    int result = pthread_cond_timedwait(condPtr, mutex, &to);
    int timedOut = 0;
    if (result == ETIMEDOUT) {
        timedOut = 1;
    } else if (result != 0) {
        printf("ERROR: pthread_cond_timedwait response was %i\n", result);
    }
    return timedOut;
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Condition_signal__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Condition_signal__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_Condition_broadcast__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_Condition_broadcast__]
    pthread_cond_t* condPtr = getConditionPtr(me);
    int result = pthread_cond_broadcast(condPtr);
    if (result != 0) {
        printf("ERROR: pthread_cond_broadcast response was %i\n", result);
    }
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_xmlvm_runtime_Condition()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex
    __TIB_org_xmlvm_runtime_Condition.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex] = 
        (VTABLE_PTR) org_xmlvm_runtime_Condition_wait___org_xmlvm_runtime_Mutex;
#endif
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long
    __TIB_org_xmlvm_runtime_Condition.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long] = 
        (VTABLE_PTR) org_xmlvm_runtime_Condition_waitOrTimeout___org_xmlvm_runtime_Mutex_long;
#endif
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_signal__
    __TIB_org_xmlvm_runtime_Condition.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_signal__] = 
        (VTABLE_PTR) org_xmlvm_runtime_Condition_signal__;
#endif
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_broadcast__
    __TIB_org_xmlvm_runtime_Condition.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_Condition_broadcast__] = 
        (VTABLE_PTR) org_xmlvm_runtime_Condition_broadcast__;
#endif
}
