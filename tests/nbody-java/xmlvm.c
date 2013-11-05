/*
 * Copyright (c) 2002-2011 by XMLVM.org
 *
 * Project Info:  http://www.xmlvm.org
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include "xmlvm.h"
#include "xmlvm-util.h"
#include "java_lang_System.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"
#include "java_lang_Throwable.h"
#include "org_xmlvm_runtime_FinalizerNotifier.h"
#include "org_xmlvm_runtime_XMLVMUtil.h"
#include <stdio.h>
#include <string.h>


XMLVM_STATIC_INITIALIZER_CONTROLLER* staticInitializerController;

// This exception value is only used for the main thread.
// Since a call to Thread.currentThread() contains try-catch blocks, this must
// be defined before the "main" java.lang.Thread is defined.
XMLVM_JMP_BUF xmlvm_exception_env_main_thread;


#ifdef XMLVM_ENABLE_STACK_TRACES


#include "uthash.h"

#define HASH_ADD_JAVA_LONG(head,javalongfield,add) \
    HASH_ADD(hh,head,javalongfield,sizeof(JAVA_LONG),add)
#define HASH_FIND_JAVA_LONG(head,findjavalong,out) \
    HASH_FIND(hh,head,findjavalong,sizeof(JAVA_LONG),out)

// A map of type UTHash with a key of JAVA_LONG and value of JAVA_OBJECT
struct hash_struct {
    JAVA_LONG key;
    JAVA_OBJECT value;
    UT_hash_handle hh; // makes this structure hashable
};


// Map of thread id to its stack trace
struct hash_struct** threadToStackTraceMapPtr;


#endif


void xmlvm_init()
{
#ifdef XMLVM_ENABLE_STACK_TRACES
    threadToStackTraceMapPtr = malloc(sizeof(struct hash_struct**));
    struct hash_struct* map = NULL; // This must be set to NULL according to the UTHash documentation
    *threadToStackTraceMapPtr = map;

    JAVA_LONG nativeThreadId = (JAVA_LONG) pthread_self();
    createStackForNewThread(nativeThreadId);
#endif

#ifndef XMLVM_NO_GC
//#ifdef DEBUG
//    setenv("GC_PRINT_STATS", "1", 1);
//#endif
    GC_INIT();
    GC_enable_incremental();
#endif

    staticInitializerController = XMLVM_MALLOC(sizeof(XMLVM_STATIC_INITIALIZER_CONTROLLER));
    staticInitializerController->initMutex = XMLVM_MALLOC(sizeof(pthread_mutex_t));
    if (0 != pthread_mutex_init(staticInitializerController->initMutex, NULL)) {
        XMLVM_ERROR("Error initializing static initializer mutex", __FILE__, __FUNCTION__, __LINE__);
    }

    __INIT_org_xmlvm_runtime_XMLVMArray();
    java_lang_Class_initNativeLayer__();
    __INIT_java_lang_System();
    org_xmlvm_runtime_XMLVMUtil_init__();
    /*
     * The implementation of the constant pool makes use of cross-compiled Java data structures.
     * During initialization of the VM done up to this point there are some circular dependencies
     * between class initializers of various classes and the constant pool that lead to some
     * inconsistencies. The easiest way to fix this is to clear the constant pool cache.
     */
    xmlvm_clear_constant_pool_cache();
    
#ifndef XMLVM_NO_GC
#ifndef EMSCRIPTEN
    GC_finalize_on_demand = 1;
    GC_java_finalization = 1;
    java_lang_Thread* finalizerThread = (java_lang_Thread*) org_xmlvm_runtime_FinalizerNotifier_startFinalizerThread__();
    GC_finalizer_notifier = org_xmlvm_runtime_FinalizerNotifier_finalizerNotifier__;
#endif
#endif

	reference_array = XMLVMUtil_NEW_ArrayList();
}

void xmlvm_destroy(java_lang_Thread* mainThread)
{
#ifdef EMSCRIPTEN
	return; // Let the JS engine handle clean up
#endif

    java_lang_Thread_threadTerminating__(mainThread);

#ifdef XMLVM_ENABLE_STACK_TRACES
    JAVA_LONG nativeThreadId = (JAVA_LONG) pthread_self();
    destroyStackForExitingThread(nativeThreadId);
#endif

    // Unregister the current thread.  Only an explicitly registered
    // thread (i.e. for which GC_register_my_thread() returns GC_SUCCESS)
    // is allowed (and required) to call this function.  (As a special
    // exception, it is also allowed to once unregister the main thread.)
#ifndef XMLVM_NO_GC
    GC_unregister_my_thread();
#endif

    // Call pthread_exit(0) so that the main thread does not terminate until
    // the other threads have finished
    pthread_exit(0);
}

/**
 * Lock a mutex. If an error occurs, terminate the program.
 */
static void lockOrExit(char* className, pthread_mutex_t* mut)
{
    int result = pthread_mutex_lock(mut);
    if (0 != result) {
        printf("Error locking mutex in %s: %i\n", className, result);
        exit(1);
    }
//    else { printf("SUCCESSFUL mutex lock in %s\n", className); }
}

/**
 * Unlock a mutex. If an error occurs, terminate the program.
 */
static void unlockOrExit(char* className, pthread_mutex_t* mut)
{
    int result = pthread_mutex_unlock(mut);
    if (0 != result) {
        printf("Error unlocking mutex in %s: %i\n", className, result);
        exit(1);
    }
//    else { printf("SUCCESSFUL mutex unlock in %s\n", className); }
}

/**
 * Lock the static initializer mutex.
 */
void staticInitializerLock(void* tibDefinition)
{
    char* className = ((struct __TIB_DEFINITION_TEMPLATE*)tibDefinition)->className;
    lockOrExit(className, staticInitializerController->initMutex);
}

/**
 * Unlock the static initializer mutex.
 */
void staticInitializerUnlock(void* tibDefinition)
{
    char* className = ((struct __TIB_DEFINITION_TEMPLATE*)tibDefinition)->className;
    unlockOrExit(className, staticInitializerController->initMutex);
}

int xmlvm_java_string_cmp(JAVA_OBJECT s1, const char* s2)
{
    java_lang_String* str = (java_lang_String*) s1;
    JAVA_INT len = str->fields.java_lang_String.count_;
    if (len != strlen(s2)) {
        return 0;
    }
    JAVA_INT offset = str->fields.java_lang_String.offset_;
    org_xmlvm_runtime_XMLVMArray* value = (org_xmlvm_runtime_XMLVMArray*) str->fields.java_lang_String.value_;
    JAVA_ARRAY_CHAR* valueArray = (JAVA_ARRAY_CHAR*) value->fields.org_xmlvm_runtime_XMLVMArray.array_;
    for (int i = 0; i < len; i++) {
        if (valueArray[i + offset] != s2[i]) {
            return 0;
        }
    }
    return 1;
}

const char* xmlvm_java_string_to_const_char(JAVA_OBJECT s)
{
    if (s == JAVA_NULL) {
        return "null";
    }
    java_lang_String* str = (java_lang_String*) s;
    JAVA_INT len = str->fields.java_lang_String.count_;
    char* cs = XMLVM_ATOMIC_MALLOC(len + 1);
    JAVA_INT offset = str->fields.java_lang_String.offset_;
    org_xmlvm_runtime_XMLVMArray* value = (org_xmlvm_runtime_XMLVMArray*) str->fields.java_lang_String.value_;
    JAVA_ARRAY_CHAR* valueArray = (JAVA_ARRAY_CHAR*) value->fields.org_xmlvm_runtime_XMLVMArray.array_;
    int i = 0;
    for (i = 0; i < len; i++) {
        cs[i] = valueArray[i + offset];
    }
    cs[i] = '\0';
    return cs;
}

JAVA_OBJECT xmlvm_create_java_string(const char* s)
{
    java_lang_String* str = __NEW_java_lang_String();
    org_xmlvm_runtime_XMLVMArray* charArray = XMLVMArray_createFromString(s);
    java_lang_String___INIT____char_1ARRAY(str, charArray);
    return XMLVMUtil_getFromStringPool(str);
}

JAVA_OBJECT xmlvm_create_java_string_array(int count, const char **s) 
{
    JAVA_OBJECT javaStrings[count];
    for (int i = 0; i < count; i++) {
        javaStrings[i] = xmlvm_create_java_string(s[i]);
    }
    JAVA_OBJECT javaStringArray = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, count);
    XMLVMArray_fillArray(javaStringArray, javaStrings);
    return javaStringArray;
}

static JAVA_OBJECT* stringConstants = JAVA_NULL;

JAVA_OBJECT xmlvm_create_java_string_from_pool(int pool_id)
{
    if (stringConstants == JAVA_NULL) {
        // TODO: use XMLVM_ATOMIC_MALLOC?
        stringConstants = XMLVM_MALLOC(xmlvm_constant_pool_size * sizeof(JAVA_OBJECT));
        XMLVM_BZERO(stringConstants, xmlvm_constant_pool_size * sizeof(JAVA_OBJECT));
    }
    if (stringConstants[pool_id] != JAVA_NULL) {
        return stringConstants[pool_id];
    }
    java_lang_String* str = __NEW_java_lang_String();
    org_xmlvm_runtime_XMLVMArray* charArray = XMLVMArray_createSingleDimensionWithData(__CLASS_char,
                                                                                       xmlvm_constant_pool_length[pool_id],
                                                                                       (JAVA_OBJECT) xmlvm_constant_pool_data[pool_id]);
    java_lang_String___INIT____char_1ARRAY(str, charArray);
    JAVA_OBJECT poolStr = XMLVMUtil_getFromStringPool(str);
    stringConstants[pool_id] = poolStr;
    return poolStr;
}

void xmlvm_clear_constant_pool_cache()
{
    XMLVM_BZERO(stringConstants, xmlvm_constant_pool_size * sizeof(JAVA_OBJECT));
}


//---------------------------------------------------------------------------------------------
// XMLVMClass

JAVA_OBJECT XMLVM_CREATE_CLASS_OBJECT(void* tib)
{
    JAVA_OBJECT clazz = __NEW_java_lang_Class();
    java_lang_Class___INIT____java_lang_Object(clazz, tib);
    return clazz;
}


JAVA_OBJECT XMLVM_CREATE_ARRAY_CLASS_OBJECT(JAVA_OBJECT baseType)
{
    __TIB_DEFINITION_org_xmlvm_runtime_XMLVMArray* tib = XMLVM_MALLOC(sizeof(__TIB_DEFINITION_org_xmlvm_runtime_XMLVMArray));
    XMLVM_MEMCPY(tib, &__TIB_org_xmlvm_runtime_XMLVMArray, sizeof(__TIB_DEFINITION_org_xmlvm_runtime_XMLVMArray));
    tib->flags = XMLVM_TYPE_ARRAY;
    tib->baseType = baseType;
    tib->arrayType = JAVA_NULL;
    JAVA_OBJECT clazz = __NEW_java_lang_Class();
    java_lang_Class___INIT____java_lang_Object(clazz, tib);
    tib->clazz = clazz;
    // Set the arrayType in in baseType to this newly created array type class
    java_lang_Class* baseTypeClass = (java_lang_Class*) baseType;
    __TIB_DEFINITION_TEMPLATE* baseTypeTIB = (__TIB_DEFINITION_TEMPLATE*) baseTypeClass->fields.java_lang_Class.tib_;
    baseTypeTIB->arrayType = clazz;
    return clazz;
}


int XMLVM_ISA(JAVA_OBJECT obj, JAVA_OBJECT clazz)
{
    if (obj == JAVA_NULL) {
        return 0;
    }
    
    int dimension_tib1 = 0;
    int dimension_tib2 = 0;
    __TIB_DEFINITION_TEMPLATE* tib1 = (__TIB_DEFINITION_TEMPLATE*) ((java_lang_Object*) obj)->tib;
    __TIB_DEFINITION_TEMPLATE* tib2 = (__TIB_DEFINITION_TEMPLATE*) ((java_lang_Class*) clazz)->fields.java_lang_Class.tib_;
    
    if (tib1 == &__TIB_org_xmlvm_runtime_XMLVMArray) {
        java_lang_Class* clazz = ((org_xmlvm_runtime_XMLVMArray*) obj)->fields.org_xmlvm_runtime_XMLVMArray.type_;
        tib1 = clazz->fields.java_lang_Class.tib_;
    }
    
    while (tib1->baseType != JAVA_NULL) {
        tib1 = ((java_lang_Class*) tib1->baseType)->fields.java_lang_Class.tib_;
        dimension_tib1++;
    }
    
    while (tib2->baseType != JAVA_NULL) {
        tib2 = ((java_lang_Class*) tib2->baseType)->fields.java_lang_Class.tib_;
        dimension_tib2++;
    }
    
    if (dimension_tib1 < dimension_tib2) {
        return 0;
    }
    
    while (tib1 != JAVA_NULL) {
        if (tib1 == tib2) {
            return 1;
        }
        // Check all implemented interfaces
        int i;
        for (i = 0; i < tib1->numImplementedInterfaces; i++) {
            if (tib1->implementedInterfaces[0][i] == tib2) {
                return 1;
            }
        }
        tib1 = tib1->extends;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------
// Stack traces

#ifdef XMLVM_ENABLE_STACK_TRACES

void createStackForNewThread(JAVA_LONG threadId)
{
    struct hash_struct *s = malloc(sizeof(struct hash_struct));
    s->key = threadId;

    XMLVM_STACK_TRACE_CURRENT* newStack = malloc(sizeof(XMLVM_STACK_TRACE_CURRENT));
    newStack->stackSize = 0;
    newStack->topOfStack = NULL;

    s->value = newStack;
    HASH_ADD_JAVA_LONG((struct hash_struct *)*threadToStackTraceMapPtr, key, s);
}

void destroyStackForExitingThread(JAVA_LONG threadId)
{
    struct hash_struct *s;
    HASH_FIND_JAVA_LONG((struct hash_struct *)*threadToStackTraceMapPtr, &threadId, s);
    if (s == NULL) {
        printf("ERROR: Unable to destroy stack trace for exiting thread!\n");
        exit(-1);
    } else {
        HASH_DEL((struct hash_struct *)*threadToStackTraceMapPtr, s);
        free(s->value);
        free(s);
    }
}

XMLVM_STACK_TRACE_CURRENT* getCurrentStackTrace()
{
	JAVA_LONG currentThreadId = (JAVA_LONG)pthread_self();
    struct hash_struct *s;
    HASH_FIND_JAVA_LONG((struct hash_struct *)*threadToStackTraceMapPtr, &currentThreadId, s);
    if (s == NULL) {
        printf("ERROR: Unable to find stack trace for current thread!\n");
        exit(-1);
    }
    return (XMLVM_STACK_TRACE_CURRENT*)s->value;
}

void xmlvmEnterMethod(XMLVM_STACK_TRACE_CURRENT* threadStack, const char* className, const char* methodName, const char* fileName)
{
    //printf("Entering method %s\n", className);

    XMLVM_STACK_TRACE_ELEMENT* newLocationElement = malloc(sizeof(XMLVM_STACK_TRACE_ELEMENT));
    newLocationElement->className = className;
    newLocationElement->methodName = methodName;
    newLocationElement->fileName = fileName;
    newLocationElement->lineNumber = -2;

    XMLVM_STACK_TRACE_LINK* link = malloc(sizeof(XMLVM_STACK_TRACE_LINK));
    link->nextLink = threadStack->topOfStack;
    if (threadStack->topOfStack != NULL) {
        link->element = threadStack->topOfStack->currentLocation;
    }
    link->currentLocation = newLocationElement;

    // Push what was the current location onto the stack and set the new current location
    threadStack->stackSize++;
    threadStack->topOfStack = link;
}

void xmlvmSourcePosition(XMLVM_STACK_TRACE_CURRENT* threadStack, const char* fileName, int lineNumber)
{
    //printf("Source position update %i\n", lineNumber);

    threadStack->topOfStack->currentLocation->fileName = fileName;
    threadStack->topOfStack->currentLocation->lineNumber = lineNumber;
}

void xmlvmExitMethod(XMLVM_STACK_TRACE_CURRENT* threadStack)
{
    //printf("Exiting method\n");

    XMLVM_STACK_TRACE_LINK* linkToDestroy = threadStack->topOfStack;
    threadStack->topOfStack = linkToDestroy->nextLink;
    threadStack->stackSize--;

    free(linkToDestroy->currentLocation);
    free(linkToDestroy);
}

void xmlvmUnwindException(XMLVM_STACK_TRACE_CURRENT* threadStack, int unwindToStackSize)
{
    while (unwindToStackSize + 1 < threadStack->stackSize) {
        //printf("Unwinding stack after catching an exception: %i > %i\n", unwindToStackSize, threadStack->stackSize);
        xmlvmExitMethod(threadStack);
    }
}

#endif


#ifdef XMLVM_ENABLE_CLASS_LOGGING
//---------------------------------------------------------------------------------------------
// Reflection/Class Usage logging

FILE *logFile    = 0;
int   useLogging = 1;

void xmlvmClassUsed(const char *prefix, const char *className) {
	if (useLogging && (logFile == 0)) {
		logFile = fopen("touched_classes.txt", "w");
	}
	if (useLogging && (logFile != 0)) {
		fprintf(logFile, "%s:%s\n", prefix, className);
	} else {
		useLogging = 0; // Failed to open file, so stop
	}
}

#endif

//---------------------------------------------------------------------------------------------
// XMLVMArray


JAVA_OBJECT XMLVMArray_createSingleDimension(JAVA_OBJECT type, JAVA_INT size)
{
    return org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(type, size);
}

JAVA_OBJECT XMLVMArray_createSingleDimensionWithData(JAVA_OBJECT type, JAVA_INT size, JAVA_OBJECT data)
{
    return org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(type, size, data);
}


JAVA_OBJECT XMLVMArray_createMultiDimensions(JAVA_OBJECT type, JAVA_OBJECT dimensions)
{
    return org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray(type, dimensions);
}

JAVA_OBJECT XMLVMArray_createFromString(const char* str)
{
    int len = strlen(str);
    int size = len * sizeof(JAVA_ARRAY_CHAR);
    int i;    
    JAVA_ARRAY_CHAR* data = XMLVM_ATOMIC_MALLOC(size);
    for (i = 0; i < len; i++) {
        data[i] = *str++;
    }
    return XMLVMArray_createSingleDimensionWithData(__CLASS_char, len, data);
}

void XMLVMArray_fillArray(JAVA_OBJECT array, void* data)
{
    org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object(array, data);
}

int XMLVMArray_count(JAVA_OBJECT array)
{
    org_xmlvm_runtime_XMLVMArray* a = (org_xmlvm_runtime_XMLVMArray*) array;
    return a->fields.org_xmlvm_runtime_XMLVMArray.length_;
}

void xmlvm_unhandled_exception()
{
    java_lang_Thread* curThread;
    curThread = (java_lang_Thread*) java_lang_Thread_currentThread__();
    JAVA_OBJECT exception = curThread->fields.java_lang_Thread.xmlvmException_;

    JAVA_OBJECT thread_name;
#ifdef XMLVM_VTABLE_IDX_java_lang_Thread_getName__
    thread_name =  ((Func_OO) ((java_lang_Thread*) curThread)->tib->vtable[XMLVM_VTABLE_IDX_java_lang_Thread_getName__])(curThread);
#else
    thread_name = java_lang_Thread_getName__(curThread);
#endif

#ifdef XMLVM_ENABLE_STACK_TRACES

    printf("Exception in thread \"%s\" ",
            xmlvm_java_string_to_const_char(thread_name));
    java_lang_Throwable_printStackTrace__(exception);

#else

    JAVA_OBJECT message;
#ifdef XMLVM_VTABLE_IDX_java_lang_Throwable_getMessage__
    message = ((Func_OO) ((java_lang_Throwable*) exception)->tib->vtable[XMLVM_VTABLE_IDX_java_lang_Throwable_getMessage__])(exception);
#else
    message = java_lang_Throwable_getMessage__(exception);
#endif

    JAVA_OBJECT exception_class;
#ifdef XMLVM_VTABLE_IDX_java_lang_Object_getClass__
    exception_class = ((Func_OO) ((java_lang_Object*) exception)->tib->vtable[XMLVM_VTABLE_IDX_java_lang_Object_getClass__])(exception);
#else
    exception_class = java_lang_Object_getClass__(exception);
#endif

    JAVA_OBJECT class_name;
#ifdef XMLVM_VTABLE_IDX_java_lang_Class_getName__
    class_name = ((Func_OO) ((java_lang_Class*) exception_class)->tib->vtable[XMLVM_VTABLE_IDX_java_lang_Class_getName__])(exception_class);
#else
    class_name = java_lang_Class_getName__(exception_class);
#endif
    
    printf("Exception in thread \"%s\" %s: %s\n",
            xmlvm_java_string_to_const_char(thread_name),
            xmlvm_java_string_to_const_char(class_name),
            xmlvm_java_string_to_const_char(message));

#endif
}

void xmlvm_unimplemented_native_method()
{
    XMLVM_ERROR("Unimplemented native method", __FILE__, __FUNCTION__, __LINE__);
}

void XMLVM_ERROR(const char* msg, const char* file, const char* function, int line)
{
    printf("XMLVM Error: %s: (%s):%s:%d\n", msg, function, file, line);
    exit(-1);
}

