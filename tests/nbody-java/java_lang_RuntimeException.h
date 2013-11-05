#ifndef __JAVA_LANG_RUNTIMEEXCEPTION__
#define __JAVA_LANG_RUNTIMEEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_RuntimeException 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Exception.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
// Class declarations for java.lang.RuntimeException
XMLVM_DEFINE_CLASS(java_lang_RuntimeException, 8, XMLVM_ITABLE_SIZE_java_lang_RuntimeException)

extern JAVA_OBJECT __CLASS_java_lang_RuntimeException;
extern JAVA_OBJECT __CLASS_java_lang_RuntimeException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_RuntimeException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_RuntimeException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_RuntimeException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_RuntimeException \
    __INSTANCE_FIELDS_java_lang_Exception; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_RuntimeException \
    } java_lang_RuntimeException

struct java_lang_RuntimeException {
    __TIB_DEFINITION_java_lang_RuntimeException* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_RuntimeException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_RuntimeException
#define XMLVM_FORWARD_DECL_java_lang_RuntimeException
typedef struct java_lang_RuntimeException java_lang_RuntimeException;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_RuntimeException 8

void __INIT_java_lang_RuntimeException();
void __INIT_IMPL_java_lang_RuntimeException();
void __DELETE_java_lang_RuntimeException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_RuntimeException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_RuntimeException();
JAVA_OBJECT __NEW_INSTANCE_java_lang_RuntimeException();
JAVA_LONG java_lang_RuntimeException_GET_serialVersionUID();
void java_lang_RuntimeException_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_RuntimeException___INIT___(JAVA_OBJECT me);
void java_lang_RuntimeException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_RuntimeException___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_RuntimeException___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
