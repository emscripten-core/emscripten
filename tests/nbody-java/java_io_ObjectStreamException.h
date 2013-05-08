#ifndef __JAVA_IO_OBJECTSTREAMEXCEPTION__
#define __JAVA_IO_OBJECTSTREAMEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_ObjectStreamException 0
// Implemented interfaces:
// Super Class:
#include "java_io_IOException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
// Class declarations for java.io.ObjectStreamException
XMLVM_DEFINE_CLASS(java_io_ObjectStreamException, 8, XMLVM_ITABLE_SIZE_java_io_ObjectStreamException)

extern JAVA_OBJECT __CLASS_java_io_ObjectStreamException;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_ObjectStreamException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_ObjectStreamException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_ObjectStreamException \
    __INSTANCE_FIELDS_java_io_IOException; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_ObjectStreamException \
    } java_io_ObjectStreamException

struct java_io_ObjectStreamException {
    __TIB_DEFINITION_java_io_ObjectStreamException* tib;
    struct {
        __INSTANCE_FIELDS_java_io_ObjectStreamException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_ObjectStreamException
#define XMLVM_FORWARD_DECL_java_io_ObjectStreamException
typedef struct java_io_ObjectStreamException java_io_ObjectStreamException;
#endif

#define XMLVM_VTABLE_SIZE_java_io_ObjectStreamException 8

void __INIT_java_io_ObjectStreamException();
void __INIT_IMPL_java_io_ObjectStreamException();
void __DELETE_java_io_ObjectStreamException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_ObjectStreamException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_ObjectStreamException();
JAVA_OBJECT __NEW_INSTANCE_java_io_ObjectStreamException();
JAVA_LONG java_io_ObjectStreamException_GET_serialVersionUID();
void java_io_ObjectStreamException_PUT_serialVersionUID(JAVA_LONG v);
void java_io_ObjectStreamException___INIT___(JAVA_OBJECT me);
void java_io_ObjectStreamException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
