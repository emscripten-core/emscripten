#ifndef __JAVA_LANG_THROWABLE__
#define __JAVA_LANG_THROWABLE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Throwable 0
// Implemented interfaces:
#include "java_io_Serializable.h"
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
// Class declarations for java.lang.Throwable
XMLVM_DEFINE_CLASS(java_lang_Throwable, 8, XMLVM_ITABLE_SIZE_java_lang_Throwable)

extern JAVA_OBJECT __CLASS_java_lang_Throwable;
extern JAVA_OBJECT __CLASS_java_lang_Throwable_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Throwable_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Throwable_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Throwable
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Throwable \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT detailMessage_; \
        JAVA_OBJECT cause_; \
        JAVA_OBJECT stackTrace_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Throwable \
    } java_lang_Throwable

struct java_lang_Throwable {
    __TIB_DEFINITION_java_lang_Throwable* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Throwable;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
typedef struct java_lang_Throwable java_lang_Throwable;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Throwable 8
#define XMLVM_VTABLE_IDX_java_lang_Throwable_getMessage__ 7
#define XMLVM_VTABLE_IDX_java_lang_Throwable_toString__ 5
#define XMLVM_VTABLE_IDX_java_lang_Throwable_getCause__ 6

void __INIT_java_lang_Throwable();
void __INIT_IMPL_java_lang_Throwable();
void __DELETE_java_lang_Throwable(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Throwable(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Throwable();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Throwable();
void xmlvm_init_native_java_lang_Throwable();
JAVA_LONG java_lang_Throwable_GET_serialVersionUID();
void java_lang_Throwable_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_Throwable___INIT___(JAVA_OBJECT me);
void java_lang_Throwable___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Throwable___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Throwable___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Throwable_fillInStackTrace__(JAVA_OBJECT me);
// Vtable index: 7
JAVA_OBJECT java_lang_Throwable_getMessage__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Throwable_getLocalizedMessage__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Throwable_getStackTraceImpl__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Throwable_getStackTrace__(JAVA_OBJECT me);
void java_lang_Throwable_setStackTrace___java_lang_StackTraceElement_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Throwable_printStackTrace__(JAVA_OBJECT me);
JAVA_INT java_lang_Throwable_countDuplicates___java_lang_StackTraceElement_1ARRAY_java_lang_StackTraceElement_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Throwable_getInternalStackTrace__(JAVA_OBJECT me);
void java_lang_Throwable_printStackTrace___java_io_PrintStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Throwable_printStackTrace___java_io_PrintWriter(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 5
JAVA_OBJECT java_lang_Throwable_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Throwable_initCause___java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 6
JAVA_OBJECT java_lang_Throwable_getCause__(JAVA_OBJECT me);
void java_lang_Throwable_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
