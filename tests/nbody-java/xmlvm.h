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

#ifndef __XMLVM_H__
#define __XMLVM_H__

#ifdef __EMSCRIPTEN__
// Workaround definitions for Emscripten
// TODO: Determine if different solution is needed

#define POLLPRI 0
#define IP_MULTICAST_IF                 32
#define IP_MULTICAST_TTL                33
#define IP_MULTICAST_LOOP               34
#define IP_ADD_MEMBERSHIP               35
#define IP_DROP_MEMBERSHIP              36
#define SO_OOBINLINE                 0

#ifndef __linux__
#define __linux__
#endif

#endif

#ifdef __linux__
#define LINUX
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <pthread.h>
#if __OBJC__ || MACOS
#include <libkern/OSAtomic.h>
#endif

#if __OBJC__ || MACOS
#define XMLVM_SPINLOCK_T OSSpinLock
#define XMLVM_SPINLOCK_INIT(spin) spin = 0
#define XMLVM_SPINLOCK_LOCK(spin) OSSpinLockLock(&spin)
#define XMLVM_SPINLOCK_UNLOCK(spin) OSSpinLockUnlock(&spin)
#else
#define XMLVM_SPINLOCK_T pthread_spinlock_t
#define XMLVM_SPINLOCK_INIT(spin) pthread_spin_init(&spin, 0)
#define XMLVM_SPINLOCK_LOCK(spin) pthread_spin_lock(&spin)
#define XMLVM_SPINLOCK_UNLOCK(spin) pthread_spin_unlock(&spin)
#endif

#ifdef DEBUG

#define XMLVM_ENABLE_STACK_TRACES
#define XMLVM_ENABLE_NPE_CHECK
#define XMLVM_ENABLE_ARRAY_BOUNDS_CHECK

#endif

#ifdef XMLVM_NO_GC

#define XMLVM_MALLOC(size) malloc(size)
#define XMLVM_ATOMIC_MALLOC(size) malloc(size)
#define XMLVM_FREE(pointer) free(pointer)
#define XMLVM_FINALIZE(me, func)
#define XMLVM_WEAK_REF(ptr)

#else

#include "gc.h"

#define XMLVM_MALLOC(size) GC_MALLOC(size)
#define XMLVM_ATOMIC_MALLOC(size) GC_MALLOC_ATOMIC(size)
#define XMLVM_FREE(pointer) GC_FREE(pointer)
#define XMLVM_FINALIZE(me, func) GC_REGISTER_FINALIZER_NO_ORDER((void *)me, func, (void *)NULL, (GC_finalization_proc *)0, (void * *)0);
//#define XMLVM_WEAK_REF(ptr) GC_register_disappearing_link(ptr)
#define XMLVM_WEAK_REF(ptr)

#endif


#define XMLVM_BZERO(pointer, size) memset((pointer), 0, size)
#define XMLVM_MEMCPY(dest, src, size) memcpy(dest, src, size)
#define XMLVM_OFFSETOF(type, field) ((unsigned long) &(((type *) 0)->field))


#define XMLVM_CLASS_INIT(class) \
    if (!__TIB_ ##class.classInitialized) __INIT_ ##class();

#define XMLVM_FORWARD_DECL(class) \
    JAVA_OBJECT __NEW_ ##class(); \
    struct class; \
    typedef struct class class; \
    extern JAVA_OBJECT __CLASS_ ##class;


void staticInitializerLock(void* tibDefinition);
void staticInitializerUnlock(void* tibDefinition);

typedef struct XMLVM_STATIC_INITIALIZER_CONTROLLER {
    pthread_mutex_t* initMutex; // a mutex locked while statically initalizing a class or classes
} XMLVM_STATIC_INITIALIZER_CONTROLLER;

typedef void               JAVA_VOID;
typedef int                JAVA_BOOLEAN;
typedef int                JAVA_CHAR;
typedef int                JAVA_BYTE;
typedef int                JAVA_SHORT;
typedef int                JAVA_INT;
typedef unsigned int       JAVA_UINT;
typedef long long          JAVA_LONG;
typedef unsigned long long JAVA_ULONG;
typedef float              JAVA_FLOAT;
typedef double             JAVA_DOUBLE;
typedef void*              JAVA_OBJECT;

//TODO which values should we use for Double.INFINITY?
#define Infinity INFINITY
#define NaN NAN

typedef char              JAVA_ARRAY_BYTE;
typedef char              JAVA_ARRAY_BOOLEAN;
typedef unsigned short    JAVA_ARRAY_CHAR;
typedef short             JAVA_ARRAY_SHORT;
typedef int               JAVA_ARRAY_INT;
typedef long long         JAVA_ARRAY_LONG;
typedef float             JAVA_ARRAY_FLOAT;
typedef double            JAVA_ARRAY_DOUBLE;
typedef JAVA_OBJECT       JAVA_ARRAY_OBJECT;


typedef union {
    JAVA_OBJECT  o;
    JAVA_INT     i;
    JAVA_FLOAT   f;
    JAVA_DOUBLE  d;
    JAVA_LONG    l;
} XMLVMElem;


extern const JAVA_ARRAY_CHAR* xmlvm_constant_pool_data[];
extern const JAVA_INT xmlvm_constant_pool_length[];
extern int xmlvm_constant_pool_size;

#define JAVA_NULL ((JAVA_OBJECT) 0)

typedef void (*VTABLE_PTR)();
typedef void (*Func_V)();
typedef void (*Func_VO)(JAVA_OBJECT o1);
typedef void (*Func_VOO)(JAVA_OBJECT o1, JAVA_OBJECT o2);
typedef void (*Func_VOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3);
typedef void (*Func_VOOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_OBJECT o4);
typedef void (*Func_VOOOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_OBJECT o4, JAVA_OBJECT o5);
typedef void (*Func_VOOOOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_OBJECT o4, JAVA_OBJECT o5, JAVA_OBJECT o6);
typedef void (*Func_VOB)(JAVA_OBJECT o1, JAVA_BOOLEAN o3);
typedef void (*Func_VOI)(JAVA_OBJECT o1, JAVA_INT i1);
typedef void (*Func_VOOB)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_BOOLEAN o3);
typedef void (*Func_VOOI)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_INT o3);
typedef void (*Func_VOID)(JAVA_OBJECT o1, JAVA_INT i1, JAVA_DOUBLE d1);

typedef JAVA_BOOLEAN (*Func_BOI)(JAVA_OBJECT o1, JAVA_INT i1);

typedef JAVA_OBJECT (*Func_O)();
typedef JAVA_OBJECT (*Func_OO)(JAVA_OBJECT o1);
typedef JAVA_OBJECT (*Func_OOO)(JAVA_OBJECT o1, JAVA_OBJECT o2);
typedef JAVA_OBJECT (*Func_OOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3);
typedef JAVA_FLOAT (*Func_FOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3);
typedef JAVA_INT (*Func_IO)(JAVA_OBJECT o1);
typedef JAVA_INT (*Func_IOO)(JAVA_OBJECT o1, JAVA_OBJECT o2);
typedef JAVA_INT (*Func_IOI)(JAVA_OBJECT o1, JAVA_INT i1);
typedef JAVA_INT (*Func_IOOI)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_INT i1);
typedef JAVA_OBJECT (*Func_OOOI)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_INT i1);
typedef JAVA_OBJECT (*Func_OOII)(JAVA_OBJECT o1, JAVA_INT i1, JAVA_INT i2);
typedef JAVA_BOOLEAN (*Func_BO)(JAVA_OBJECT o1);
typedef JAVA_BOOLEAN (*Func_BOO)(JAVA_OBJECT o1, JAVA_OBJECT o2);
typedef JAVA_BOOLEAN (*Func_BOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3);
typedef JAVA_BOOLEAN (*Func_BOOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_OBJECT o4);
typedef JAVA_BOOLEAN (*Func_BOOOOO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_OBJECT o4, JAVA_OBJECT o5);
typedef JAVA_BOOLEAN (*Func_BOOOI)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_OBJECT o3, JAVA_INT o4);
typedef void (*Func_VOOIO)(JAVA_OBJECT o1, JAVA_OBJECT o2, JAVA_INT i1, JAVA_OBJECT o3);

#define java_lang_reflect_Modifier_PUBLIC       1
#define java_lang_reflect_Modifier_PRIVATE      2
#define java_lang_reflect_Modifier_PROTECTED    4
#define java_lang_reflect_Modifier_STATIC       8
#define java_lang_reflect_Modifier_FINAL        16
#define java_lang_reflect_Modifier_SYNCHRONIZED 32
#define java_lang_reflect_Modifier_VOLATILE     64
#define java_lang_reflect_Modifier_TRANSIENT    128
#define java_lang_reflect_Modifier_NATIVE       256
#define java_lang_reflect_Modifier_INTERFACE    512
#define java_lang_reflect_Modifier_ABSTRACT     1024
#define java_lang_reflect_Modifier_STRICT       2048
#define java_lang_reflect_Modifier_BRIDGE       64
#define java_lang_reflect_Modifier_VARARGS      128
#define java_lang_reflect_Modifier_SYNTHETIC    4096
#define java_lang_reflect_Modifier_ANNOTATION   8192
#define java_lang_reflect_Modifier_ENUM         16384

typedef struct {
    const char*  name;
    JAVA_OBJECT* type;
    JAVA_INT     modifiers;
    JAVA_INT     offset;
    JAVA_OBJECT* address;
    const char*  signature;
    JAVA_OBJECT  annotations; // XMLVMArray(byte)
} XMLVM_FIELD_REFLECTION_DATA;

typedef struct {
    JAVA_OBJECT** parameterTypes;
    int           numParameterTypes;
    JAVA_OBJECT*  checkedExceptions;
    int           numCheckedExceptions;
    int           modifiers;
    const char*   signature;
    JAVA_OBJECT   annotations;
    JAVA_OBJECT   parameterAnnotations;
} XMLVM_CONSTRUCTOR_REFLECTION_DATA;

typedef struct {
    const char*   name;
    JAVA_OBJECT** parameterTypes;
    int           numParameterTypes;
    JAVA_OBJECT*  checkedExceptions;
    int           numCheckedExceptions;
    int           modifiers;
    const char*   signature;
    JAVA_OBJECT   annotations;
    JAVA_OBJECT   parameterAnnotations;
} XMLVM_METHOD_REFLECTION_DATA;

#define XMLVM_TYPE_CLASS     1
#define XMLVM_TYPE_INTERFACE 2
#define XMLVM_TYPE_ENUM      4
#define XMLVM_TYPE_PRIMITIVE 8
#define XMLVM_TYPE_ARRAY     16

#define XMLVM_DEFINE_CLASS(name, vtableSize, itableSize) \
typedef struct __TIB_DEFINITION_##name { \
    int                                 classInitializationBegan; \
    int                                 classInitialized; \
    JAVA_LONG                           initializerThreadId; \
    Func_V                              classInitializer; \
    const char*                         className; \
    const char*                         packageName; \
    const char*                         enclosingClassName; \
    const char*                         enclosingMethodName; \
    const char*                         signature; \
    struct __TIB_DEFINITION_TEMPLATE*   extends; \
    int                                 sizeInstance; \
    int                                 flags; \
    JAVA_OBJECT                         clazz; \
    JAVA_OBJECT                         baseType; \
    JAVA_OBJECT                         arrayType; \
    XMLVM_FIELD_REFLECTION_DATA*        declaredFields; \
    int                                 numDeclaredFields; \
    XMLVM_CONSTRUCTOR_REFLECTION_DATA*  declaredConstructors; \
    int                                 numDeclaredConstructors; \
    Func_OOO                            constructorDispatcherFunc; \
    XMLVM_METHOD_REFLECTION_DATA*       declaredMethods; \
    int                                 numDeclaredMethods; \
    Func_OOOO                           methodDispatcherFunc; \
    Func_O                              newInstanceFunc; \
    int                                 numInterfaces; \
    struct __TIB_DEFINITION_TEMPLATE* (*interfaces)[1]; \
    int                                 numImplementedInterfaces; \
    struct __TIB_DEFINITION_TEMPLATE* (*implementedInterfaces)[1]; \
    VTABLE_PTR*                         itableBegin; \
    VTABLE_PTR                          vtable[vtableSize]; \
    VTABLE_PTR                          itable[itableSize]; \
} __TIB_DEFINITION_##name; \
\
extern __TIB_DEFINITION_##name __TIB_##name;

XMLVM_DEFINE_CLASS(TEMPLATE, 0, 0)

int XMLVM_ISA(JAVA_OBJECT obj, JAVA_OBJECT clazz);
int xmlvm_java_string_cmp(JAVA_OBJECT s1, const char* s2);
const char* xmlvm_java_string_to_const_char(JAVA_OBJECT s);
JAVA_OBJECT xmlvm_create_java_string(const char* s);
JAVA_OBJECT xmlvm_create_java_string_array(int count, const char **s);
JAVA_OBJECT xmlvm_create_java_string_from_pool(int pool_id);
void xmlvm_clear_constant_pool_cache();

#define XMLVM_SIZE_OF_OBJECT_VTABLE 11


//---------------------------------------------------------------------------------------------
// XMLVMClass

// Generated by AugmentedCOutputProcess in file xmlvm-tib-list.c and used in Class.forName()
extern __TIB_DEFINITION_TEMPLATE* __xmlvm_tib_list[];
extern int __xmlvm_num_tib;

extern JAVA_OBJECT __CLASS_boolean;
extern JAVA_OBJECT __CLASS_byte;
extern JAVA_OBJECT __CLASS_char;
extern JAVA_OBJECT __CLASS_short;
extern JAVA_OBJECT __CLASS_int;
extern JAVA_OBJECT __CLASS_long;
extern JAVA_OBJECT __CLASS_float;
extern JAVA_OBJECT __CLASS_double;
extern JAVA_OBJECT __CLASS_void;

extern JAVA_OBJECT __CLASS_boolean_1ARRAY;
extern JAVA_OBJECT __CLASS_byte_1ARRAY;
extern JAVA_OBJECT __CLASS_char_1ARRAY;
extern JAVA_OBJECT __CLASS_short_1ARRAY;
extern JAVA_OBJECT __CLASS_int_1ARRAY;
extern JAVA_OBJECT __CLASS_long_1ARRAY;
extern JAVA_OBJECT __CLASS_float_1ARRAY;
extern JAVA_OBJECT __CLASS_double_1ARRAY;

extern JAVA_OBJECT __CLASS_boolean_2ARRAY;
extern JAVA_OBJECT __CLASS_byte_2ARRAY;
extern JAVA_OBJECT __CLASS_char_2ARRAY;
extern JAVA_OBJECT __CLASS_short_2ARRAY;
extern JAVA_OBJECT __CLASS_int_2ARRAY;
extern JAVA_OBJECT __CLASS_long_2ARRAY;
extern JAVA_OBJECT __CLASS_float_2ARRAY;
extern JAVA_OBJECT __CLASS_double_2ARRAY;

extern JAVA_OBJECT __CLASS_boolean_3ARRAY;
extern JAVA_OBJECT __CLASS_byte_3ARRAY;
extern JAVA_OBJECT __CLASS_char_3ARRAY;
extern JAVA_OBJECT __CLASS_short_3ARRAY;
extern JAVA_OBJECT __CLASS_int_3ARRAY;
extern JAVA_OBJECT __CLASS_long_3ARRAY;
extern JAVA_OBJECT __CLASS_float_3ARRAY;
extern JAVA_OBJECT __CLASS_double_3ARRAY;

JAVA_OBJECT XMLVM_CREATE_CLASS_OBJECT(void* tib);
JAVA_OBJECT XMLVM_CREATE_ARRAY_CLASS_OBJECT(JAVA_OBJECT baseType);

extern JAVA_OBJECT __CLASS_org_xmlvm_runtime_RedTypeMarker;


//---------------------------------------------------------------------------------------------
// XMLVMArray

JAVA_OBJECT XMLVMArray_createSingleDimension(JAVA_OBJECT type, int size);
JAVA_OBJECT XMLVMArray_createSingleDimensionWithData(JAVA_OBJECT type, int size, void* data);
JAVA_OBJECT XMLVMArray_createMultiDimensions(JAVA_OBJECT type, JAVA_OBJECT dimensions);
JAVA_OBJECT XMLVMArray_createFromString(const char* str);
void XMLVMArray_fillArray(JAVA_OBJECT array, void* data);
JAVA_INT XMLVMArray_count(JAVA_OBJECT array);

XMLVM_DEFINE_CLASS(boolean, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(byte, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(char, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(short, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(int, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(long, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(float, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(double, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(void, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)

void __INIT_boolean();
void __INIT_byte();
void __INIT_char();
void __INIT_short();
void __INIT_int();
void __INIT_long();
void __INIT_float();
void __INIT_double();
void __INIT_void();

XMLVM_DEFINE_CLASS(java_lang_Object_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(boolean_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(byte_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(char_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(short_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(int_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(long_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(float_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)
XMLVM_DEFINE_CLASS(double_ARRAYTYPE, XMLVM_SIZE_OF_OBJECT_VTABLE, 0)

#include "org_xmlvm_runtime_XMLVMArray.h"
#include "java_lang_Thread.h"

#define XMLVM_JMP_BUF jmp_buf

#ifndef XMLVM_ASM_JS
#define XMLVM_SETJMP(env) setjmp(env)
#define XMLVM_LONGJMP(env) longjmp(env, 0)
#else
#define XMLVM_SETJMP(env) 0
#define XMLVM_LONGJMP(env)
#endif

// This exception value is only used for the main thread.
// Since a call to Thread.currentThread() contains try-catch blocks, this must
// be defined before the "main" java.lang.Thread is defined.
extern XMLVM_JMP_BUF xmlvm_exception_env_main_thread;

#define XMLVM_NOT_IMPLEMENTED() XMLVM_ERROR("Not implemented", __FILE__, __FUNCTION__, __LINE__)
#define XMLVM_UNIMPLEMENTED_NATIVE_METHOD() XMLVM_ERROR("Unimplemented native method", __FILE__, __FUNCTION__, __LINE__)
#define XMLVM_INTERNAL_ERROR() XMLVM_ERROR("Internal error", __FILE__, __FUNCTION__, __LINE__)
#define XMLVM_RED_CLASS_DEPENDENCY() XMLVM_ERROR("Unsatisfied red class dependency", __FILE__, __FUNCTION__, __LINE__)

void xmlvm_unimplemented_native_method();
void xmlvm_unhandled_exception();
void XMLVM_ERROR(const char* msg, const char* file, const char* function, int line);


//---------------------------------------------------------------------------------------------
// Stack traces


#ifdef XMLVM_ENABLE_STACK_TRACES

typedef struct XMLVM_STACK_TRACE_ELEMENT {
    char* className;
    char* methodName;
    char* fileName;
    int lineNumber;
} XMLVM_STACK_TRACE_ELEMENT;

typedef struct XMLVM_STACK_TRACE_LINK {
    // "struct" is needed here since the typedef is not yet declared.
    struct XMLVM_STACK_TRACE_LINK* nextLink;
    XMLVM_STACK_TRACE_ELEMENT* element;
    XMLVM_STACK_TRACE_ELEMENT* currentLocation;
} XMLVM_STACK_TRACE_LINK;

typedef struct XMLVM_STACK_TRACE_CURRENT {
    int stackSize;
    XMLVM_STACK_TRACE_LINK* topOfStack;
} XMLVM_STACK_TRACE_CURRENT;

#define XMLVM_ENTER_METHOD(className, methodName, fileName) \
    XMLVM_STACK_TRACE_CURRENT* threadStack = getCurrentStackTrace(); \
    int threadStackSize = threadStack->stackSize; \
    xmlvmEnterMethod(threadStack, className, methodName, fileName);
#define XMLVM_SOURCE_POSITION(fileName, lineNumber) \
    xmlvmSourcePosition(threadStack, fileName, lineNumber);
#define XMLVM_EXIT_METHOD() \
    xmlvmExitMethod(threadStack);
#define XMLVM_UNWIND_EXCEPTION() \
    xmlvmUnwindException(threadStack, threadStackSize);

void createStackForNewThread(JAVA_LONG threadId);
void destroyStackForExitingThread(JAVA_LONG threadId);
XMLVM_STACK_TRACE_CURRENT* getCurrentStackTrace();
void xmlvmEnterMethod(XMLVM_STACK_TRACE_CURRENT* threadStack, const char* className, const char* methodName, const char* fileName);
void xmlvmSourcePosition(XMLVM_STACK_TRACE_CURRENT* threadStack, const char* fileName, int lineNumber);
void xmlvmExitMethod(XMLVM_STACK_TRACE_CURRENT* threadStack);
void xmlvmUnwindException(XMLVM_STACK_TRACE_CURRENT* threadStack, int unwindToStackSize);

#else

#define XMLVM_ENTER_METHOD(className, methodName, fileName)
#define XMLVM_SOURCE_POSITION(fileName, lineNumber)
#define XMLVM_EXIT_METHOD()
#define XMLVM_UNWIND_EXCEPTION()

#endif

//---------------------------------------------------------------------------------------------
// Reflection logging

#ifdef XMLVM_ENABLE_CLASS_LOGGING

void xmlvmClassUsed(const char *prefix, const char *className);

#define XMLVM_REFLECTION_USED(className) \
    xmlvmClassUsed("R", className);

#define XMLVM_CLASS_USED(className) \
    xmlvmClassUsed("C", className);

#else

#define XMLVM_REFLECTION_USED(className)
#define XMLVM_CLASS_USED(className)

#endif


//---------------------------------------------------------------------------------------------
#define XMLVM_TRY_BEGIN(uniqueId) \
    volatile XMLVM_JMP_BUF local_env_##uniqueId; \
    volatile java_lang_Thread* curThread_##uniqueId = (java_lang_Thread*)java_lang_Thread_currentThread__(); \
    XMLVM_MEMCPY(local_env_##uniqueId, curThread_##uniqueId->fields.java_lang_Thread.xmlvmExceptionEnv_, sizeof(XMLVM_JMP_BUF)); \
    if (!XMLVM_SETJMP(curThread_##uniqueId->fields.java_lang_Thread.xmlvmExceptionEnv_)) {
#define XMLVM_TRY_END }
#define XMLVM_CATCH_BEGIN(uniqueId) \
    else { \
        XMLVM_UNWIND_EXCEPTION() \
        XMLVM_MEMCPY(curThread_##uniqueId->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_##uniqueId, sizeof(XMLVM_JMP_BUF));
#define XMLVM_CATCH_SPECIFIC(uniqueId, type, target) \
        if (!__TIB_##type.classInitialized) __INIT_##type(); \
        if (XMLVM_ISA(curThread_##uniqueId->fields.java_lang_Thread.xmlvmException_, __CLASS_##type)) goto label##target;
#define XMLVM_CATCH_END(uniqueId) \
        XMLVM_LONGJMP(curThread_##uniqueId->fields.java_lang_Thread.xmlvmExceptionEnv_); \
    }
#define XMLVM_RESTORE_EXCEPTION_ENV(uniqueId) \
    XMLVM_MEMCPY(curThread_##uniqueId->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_##uniqueId, sizeof(XMLVM_JMP_BUF));

// Throw an exception that has already been initialized and constructed
#define XMLVM_THROW_CUSTOM(exception) { \
        java_lang_Thread* macroCurThread = (java_lang_Thread*)java_lang_Thread_currentThread__(); \
        macroCurThread->fields.java_lang_Thread.xmlvmException_ = exception; \
        XMLVM_LONGJMP(macroCurThread->fields.java_lang_Thread.xmlvmExceptionEnv_); \
    }
// Throw an exception which is automatically constructed with the default constructor
#define XMLVM_THROW(exceptionType) { \
        java_lang_Thread* macroCurThread = (java_lang_Thread*)java_lang_Thread_currentThread__(); \
        macroCurThread->fields.java_lang_Thread.xmlvmException_ = __NEW_##exceptionType(); \
        exceptionType##___INIT___(macroCurThread->fields.java_lang_Thread.xmlvmException_); \
        XMLVM_LONGJMP(macroCurThread->fields.java_lang_Thread.xmlvmExceptionEnv_); \
    }
// Throw an exception which is automatically constructed with a String parameter derived from the C String
#define XMLVM_THROW_WITH_CSTRING(exceptionType, errorMsg) { \
        java_lang_Thread* macroCurThread = (java_lang_Thread*)java_lang_Thread_currentThread__(); \
        macroCurThread->fields.java_lang_Thread.xmlvmException_ = __NEW_##exceptionType(); \
        exceptionType##___INIT____java_lang_String(macroCurThread->fields.java_lang_Thread.xmlvmException_, xmlvm_create_java_string(errorMsg)); \
        XMLVM_LONGJMP(macroCurThread->fields.java_lang_Thread.xmlvmExceptionEnv_); \
    }

#ifdef XMLVM_ENABLE_NPE_CHECK

#define XMLVM_CHECK_NPE(register) \
        if (_r##register.o == JAVA_NULL) { \
            XMLVM_THROW(java_lang_NullPointerException) \
        }

#else

#define XMLVM_CHECK_NPE(register)

#endif // XMLVM_ENABLE_NPE_CHECK


#ifdef XMLVM_ENABLE_ARRAY_BOUNDS_CHECK

#define XMLVM_CHECK_ARRAY_BOUNDS(arr, idx) \
        if ((idx < 0) || (idx >= ((org_xmlvm_runtime_XMLVMArray*) arr)->fields.org_xmlvm_runtime_XMLVMArray.length_)) { \
            XMLVM_THROW(java_lang_ArrayIndexOutOfBoundsException) \
        }

#else

#define XMLVM_CHECK_ARRAY_BOUNDS(arr, idx)

#endif // XMLVM_ENABLE_ARRAY_BOUNDS_CHECK


void xmlvm_init();
void xmlvm_destroy(java_lang_Thread* mainThread);

// A list of Java instances which are currently referenced by non-C types.
// This is used to avoid premature garbage collection.
JAVA_OBJECT reference_array;

#endif
