#ifndef __JAVA_LANG_NOSUCHFIELDEXCEPTION__
#define __JAVA_LANG_NOSUCHFIELDEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_NoSuchFieldException 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Exception.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.NoSuchFieldException
XMLVM_DEFINE_CLASS(java_lang_NoSuchFieldException, 8, XMLVM_ITABLE_SIZE_java_lang_NoSuchFieldException)

extern JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException;
extern JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_NoSuchFieldException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_NoSuchFieldException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_NoSuchFieldException \
    __INSTANCE_FIELDS_java_lang_Exception; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_NoSuchFieldException \
    } java_lang_NoSuchFieldException

struct java_lang_NoSuchFieldException {
    __TIB_DEFINITION_java_lang_NoSuchFieldException* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_NoSuchFieldException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_NoSuchFieldException
#define XMLVM_FORWARD_DECL_java_lang_NoSuchFieldException
typedef struct java_lang_NoSuchFieldException java_lang_NoSuchFieldException;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_NoSuchFieldException 8

void __INIT_java_lang_NoSuchFieldException();
void __INIT_IMPL_java_lang_NoSuchFieldException();
void __DELETE_java_lang_NoSuchFieldException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_NoSuchFieldException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_NoSuchFieldException();
JAVA_OBJECT __NEW_INSTANCE_java_lang_NoSuchFieldException();
JAVA_LONG java_lang_NoSuchFieldException_GET_serialVersionUID();
void java_lang_NoSuchFieldException_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_NoSuchFieldException___INIT___(JAVA_OBJECT me);
void java_lang_NoSuchFieldException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
