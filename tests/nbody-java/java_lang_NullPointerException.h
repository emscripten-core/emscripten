#ifndef __JAVA_LANG_NULLPOINTEREXCEPTION__
#define __JAVA_LANG_NULLPOINTEREXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_NullPointerException 0
// Implemented interfaces:
// Super Class:
#include "java_lang_RuntimeException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.NullPointerException
XMLVM_DEFINE_CLASS(java_lang_NullPointerException, 8, XMLVM_ITABLE_SIZE_java_lang_NullPointerException)

extern JAVA_OBJECT __CLASS_java_lang_NullPointerException;
extern JAVA_OBJECT __CLASS_java_lang_NullPointerException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_NullPointerException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_NullPointerException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_NullPointerException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_NullPointerException \
    __INSTANCE_FIELDS_java_lang_RuntimeException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_NullPointerException \
    } java_lang_NullPointerException

struct java_lang_NullPointerException {
    __TIB_DEFINITION_java_lang_NullPointerException* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_NullPointerException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
typedef struct java_lang_NullPointerException java_lang_NullPointerException;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_NullPointerException 8

void __INIT_java_lang_NullPointerException();
void __INIT_IMPL_java_lang_NullPointerException();
void __DELETE_java_lang_NullPointerException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_NullPointerException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_NullPointerException();
JAVA_OBJECT __NEW_INSTANCE_java_lang_NullPointerException();
JAVA_LONG java_lang_NullPointerException_GET_serialVersionUID();
void java_lang_NullPointerException_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_NullPointerException___INIT___(JAVA_OBJECT me);
void java_lang_NullPointerException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
