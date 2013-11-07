#ifndef __JAVA_IO_INTERRUPTEDIOEXCEPTION__
#define __JAVA_IO_INTERRUPTEDIOEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_InterruptedIOException 0
// Implemented interfaces:
// Super Class:
#include "java_io_IOException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.io.InterruptedIOException
XMLVM_DEFINE_CLASS(java_io_InterruptedIOException, 8, XMLVM_ITABLE_SIZE_java_io_InterruptedIOException)

extern JAVA_OBJECT __CLASS_java_io_InterruptedIOException;
extern JAVA_OBJECT __CLASS_java_io_InterruptedIOException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InterruptedIOException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InterruptedIOException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_InterruptedIOException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_InterruptedIOException \
    __INSTANCE_FIELDS_java_io_IOException; \
    struct { \
        JAVA_INT bytesTransferred_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_InterruptedIOException \
    } java_io_InterruptedIOException

struct java_io_InterruptedIOException {
    __TIB_DEFINITION_java_io_InterruptedIOException* tib;
    struct {
        __INSTANCE_FIELDS_java_io_InterruptedIOException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_InterruptedIOException
#define XMLVM_FORWARD_DECL_java_io_InterruptedIOException
typedef struct java_io_InterruptedIOException java_io_InterruptedIOException;
#endif

#define XMLVM_VTABLE_SIZE_java_io_InterruptedIOException 8

void __INIT_java_io_InterruptedIOException();
void __INIT_IMPL_java_io_InterruptedIOException();
void __DELETE_java_io_InterruptedIOException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_InterruptedIOException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_InterruptedIOException();
JAVA_OBJECT __NEW_INSTANCE_java_io_InterruptedIOException();
JAVA_LONG java_io_InterruptedIOException_GET_serialVersionUID();
void java_io_InterruptedIOException_PUT_serialVersionUID(JAVA_LONG v);
void java_io_InterruptedIOException___INIT___(JAVA_OBJECT me);
void java_io_InterruptedIOException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
