#ifndef __JAVA_LANG_INDEXOUTOFBOUNDSEXCEPTION__
#define __JAVA_LANG_INDEXOUTOFBOUNDSEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_IndexOutOfBoundsException 0
// Implemented interfaces:
// Super Class:
#include "java_lang_RuntimeException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.IndexOutOfBoundsException
XMLVM_DEFINE_CLASS(java_lang_IndexOutOfBoundsException, 8, XMLVM_ITABLE_SIZE_java_lang_IndexOutOfBoundsException)

extern JAVA_OBJECT __CLASS_java_lang_IndexOutOfBoundsException;
extern JAVA_OBJECT __CLASS_java_lang_IndexOutOfBoundsException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_IndexOutOfBoundsException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_IndexOutOfBoundsException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_IndexOutOfBoundsException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_IndexOutOfBoundsException \
    __INSTANCE_FIELDS_java_lang_RuntimeException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_IndexOutOfBoundsException \
    } java_lang_IndexOutOfBoundsException

struct java_lang_IndexOutOfBoundsException {
    __TIB_DEFINITION_java_lang_IndexOutOfBoundsException* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_IndexOutOfBoundsException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
typedef struct java_lang_IndexOutOfBoundsException java_lang_IndexOutOfBoundsException;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_IndexOutOfBoundsException 8

void __INIT_java_lang_IndexOutOfBoundsException();
void __INIT_IMPL_java_lang_IndexOutOfBoundsException();
void __DELETE_java_lang_IndexOutOfBoundsException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_IndexOutOfBoundsException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_IndexOutOfBoundsException();
JAVA_OBJECT __NEW_INSTANCE_java_lang_IndexOutOfBoundsException();
JAVA_LONG java_lang_IndexOutOfBoundsException_GET_serialVersionUID();
void java_lang_IndexOutOfBoundsException_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_IndexOutOfBoundsException___INIT___(JAVA_OBJECT me);
void java_lang_IndexOutOfBoundsException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
