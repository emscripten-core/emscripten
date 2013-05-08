#ifndef __JAVA_IO_INVALIDOBJECTEXCEPTION__
#define __JAVA_IO_INVALIDOBJECTEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_InvalidObjectException 0
// Implemented interfaces:
// Super Class:
#include "java_io_ObjectStreamException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.io.InvalidObjectException
XMLVM_DEFINE_CLASS(java_io_InvalidObjectException, 8, XMLVM_ITABLE_SIZE_java_io_InvalidObjectException)

extern JAVA_OBJECT __CLASS_java_io_InvalidObjectException;
extern JAVA_OBJECT __CLASS_java_io_InvalidObjectException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InvalidObjectException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InvalidObjectException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_InvalidObjectException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_InvalidObjectException \
    __INSTANCE_FIELDS_java_io_ObjectStreamException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_InvalidObjectException \
    } java_io_InvalidObjectException

struct java_io_InvalidObjectException {
    __TIB_DEFINITION_java_io_InvalidObjectException* tib;
    struct {
        __INSTANCE_FIELDS_java_io_InvalidObjectException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_InvalidObjectException
#define XMLVM_FORWARD_DECL_java_io_InvalidObjectException
typedef struct java_io_InvalidObjectException java_io_InvalidObjectException;
#endif

#define XMLVM_VTABLE_SIZE_java_io_InvalidObjectException 8

void __INIT_java_io_InvalidObjectException();
void __INIT_IMPL_java_io_InvalidObjectException();
void __DELETE_java_io_InvalidObjectException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_InvalidObjectException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_InvalidObjectException();
JAVA_OBJECT __NEW_INSTANCE_java_io_InvalidObjectException();
JAVA_LONG java_io_InvalidObjectException_GET_serialVersionUID();
void java_io_InvalidObjectException_PUT_serialVersionUID(JAVA_LONG v);
void java_io_InvalidObjectException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
