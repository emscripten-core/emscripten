#ifndef __JAVA_LANG_CLASSNOTFOUNDEXCEPTION__
#define __JAVA_LANG_CLASSNOTFOUNDEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_ClassNotFoundException 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Exception.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
// Class declarations for java.lang.ClassNotFoundException
XMLVM_DEFINE_CLASS(java_lang_ClassNotFoundException, 8, XMLVM_ITABLE_SIZE_java_lang_ClassNotFoundException)

extern JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException;
extern JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_ClassNotFoundException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_ClassNotFoundException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_ClassNotFoundException \
    __INSTANCE_FIELDS_java_lang_Exception; \
    struct { \
        JAVA_OBJECT ex_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_ClassNotFoundException \
    } java_lang_ClassNotFoundException

struct java_lang_ClassNotFoundException {
    __TIB_DEFINITION_java_lang_ClassNotFoundException* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_ClassNotFoundException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_ClassNotFoundException
#define XMLVM_FORWARD_DECL_java_lang_ClassNotFoundException
typedef struct java_lang_ClassNotFoundException java_lang_ClassNotFoundException;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_ClassNotFoundException 8
#define XMLVM_VTABLE_IDX_java_lang_ClassNotFoundException_getCause__ 6

void __INIT_java_lang_ClassNotFoundException();
void __INIT_IMPL_java_lang_ClassNotFoundException();
void __DELETE_java_lang_ClassNotFoundException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_ClassNotFoundException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_ClassNotFoundException();
JAVA_OBJECT __NEW_INSTANCE_java_lang_ClassNotFoundException();
JAVA_LONG java_lang_ClassNotFoundException_GET_serialVersionUID();
void java_lang_ClassNotFoundException_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_ClassNotFoundException___INIT___(JAVA_OBJECT me);
void java_lang_ClassNotFoundException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_ClassNotFoundException___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_ClassNotFoundException_getException__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_OBJECT java_lang_ClassNotFoundException_getCause__(JAVA_OBJECT me);

#endif
