#ifndef __JAVA_LANG_THREAD_STATE__
#define __JAVA_LANG_THREAD_STATE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Thread_State 8
// Implemented interfaces:
// Super Class:
#include "java_lang_Enum.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.Thread$State
XMLVM_DEFINE_CLASS(java_lang_Thread_State, 7, XMLVM_ITABLE_SIZE_java_lang_Thread_State)

extern JAVA_OBJECT __CLASS_java_lang_Thread_State;
extern JAVA_OBJECT __CLASS_java_lang_Thread_State_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Thread_State_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Thread_State_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Thread_State
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Thread_State \
    __INSTANCE_FIELDS_java_lang_Enum; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Thread_State \
    } java_lang_Thread_State

struct java_lang_Thread_State {
    __TIB_DEFINITION_java_lang_Thread_State* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Thread_State;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread_State
#define XMLVM_FORWARD_DECL_java_lang_Thread_State
typedef struct java_lang_Thread_State java_lang_Thread_State;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Thread_State 7

void __INIT_java_lang_Thread_State();
void __INIT_IMPL_java_lang_Thread_State();
void __DELETE_java_lang_Thread_State(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Thread_State(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Thread_State();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Thread_State();
JAVA_OBJECT java_lang_Thread_State_GET_NEW();
void java_lang_Thread_State_PUT_NEW(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET_RUNNABLE();
void java_lang_Thread_State_PUT_RUNNABLE(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET_BLOCKED();
void java_lang_Thread_State_PUT_BLOCKED(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET_WAITING();
void java_lang_Thread_State_PUT_WAITING(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET_TIMED_WAITING();
void java_lang_Thread_State_PUT_TIMED_WAITING(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET_TERMINATED();
void java_lang_Thread_State_PUT_TERMINATED(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_GET__VALUES();
void java_lang_Thread_State_PUT__VALUES(JAVA_OBJECT v);
JAVA_OBJECT java_lang_Thread_State_values__();
JAVA_OBJECT java_lang_Thread_State_valueOf___java_lang_String(JAVA_OBJECT n1);
void java_lang_Thread_State___INIT____java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2);
void java_lang_Thread_State___CLINIT_();

#endif
