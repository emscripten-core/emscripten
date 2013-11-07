#ifndef __JAVA_IO_IOEXCEPTION__
#define __JAVA_IO_IOEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_IOException 0
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
// Class declarations for java.io.IOException
XMLVM_DEFINE_CLASS(java_io_IOException, 8, XMLVM_ITABLE_SIZE_java_io_IOException)

extern JAVA_OBJECT __CLASS_java_io_IOException;
extern JAVA_OBJECT __CLASS_java_io_IOException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_IOException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_IOException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_IOException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_IOException \
    __INSTANCE_FIELDS_java_lang_Exception; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_IOException \
    } java_io_IOException

struct java_io_IOException {
    __TIB_DEFINITION_java_io_IOException* tib;
    struct {
        __INSTANCE_FIELDS_java_io_IOException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
typedef struct java_io_IOException java_io_IOException;
#endif

#define XMLVM_VTABLE_SIZE_java_io_IOException 8

void __INIT_java_io_IOException();
void __INIT_IMPL_java_io_IOException();
void __DELETE_java_io_IOException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_IOException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_IOException();
JAVA_OBJECT __NEW_INSTANCE_java_io_IOException();
JAVA_LONG java_io_IOException_GET_serialVersionUID();
void java_io_IOException_PUT_serialVersionUID(JAVA_LONG v);
void java_io_IOException___INIT___(JAVA_OBJECT me);
void java_io_IOException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_IOException___INIT____java_lang_String_java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_IOException___INIT____java_lang_Throwable(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
