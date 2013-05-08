#ifndef __JAVA_LANG_EXCEPTION__
#define __JAVA_LANG_EXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Exception 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Throwable.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.lang.Exception
XMLVM_DEFINE_CLASS(java_lang_Exception, 8, XMLVM_ITABLE_SIZE_java_lang_Exception)

extern JAVA_OBJECT __CLASS_java_lang_Exception;
extern JAVA_OBJECT __CLASS_java_lang_Exception_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Exception_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Exception_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Exception
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Exception \
    __INSTANCE_FIELDS_java_lang_Throwable; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Exception \
    } java_lang_Exception

struct java_lang_Exception {
    __TIB_DEFINITION_java_lang_Exception* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Exception;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Exception
#define XMLVM_FORWARD_DECL_java_lang_Exception
typedef struct java_lang_Exception java_lang_Exception;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Exception 8

void __INIT_java_lang_Exception();
void __INIT_IMPL_java_lang_Exception();
void __DELETE_java_lang_Exception(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Exception(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Exception();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Exception();
JAVA_LONG java_lang_Exception_GET_serialVersionUID();
void java_lang_Exception_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_Exception___INIT___(JAVA_OBJECT me);
void java_lang_Exception___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Exception___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_lang_Exception___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
