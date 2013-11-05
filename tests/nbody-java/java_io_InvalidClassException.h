#ifndef __JAVA_IO_INVALIDCLASSEXCEPTION__
#define __JAVA_IO_INVALIDCLASSEXCEPTION__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_InvalidClassException 0
// Implemented interfaces:
// Super Class:
#include "java_io_ObjectStreamException.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Object
#define XMLVM_FORWARD_DECL_java_lang_Object
XMLVM_FORWARD_DECL(java_lang_Object)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for java.io.InvalidClassException
XMLVM_DEFINE_CLASS(java_io_InvalidClassException, 8, XMLVM_ITABLE_SIZE_java_io_InvalidClassException)

extern JAVA_OBJECT __CLASS_java_io_InvalidClassException;
extern JAVA_OBJECT __CLASS_java_io_InvalidClassException_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InvalidClassException_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_InvalidClassException_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_InvalidClassException
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_InvalidClassException \
    __INSTANCE_FIELDS_java_io_ObjectStreamException; \
    struct { \
        JAVA_OBJECT classname_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_InvalidClassException \
    } java_io_InvalidClassException

struct java_io_InvalidClassException {
    __TIB_DEFINITION_java_io_InvalidClassException* tib;
    struct {
        __INSTANCE_FIELDS_java_io_InvalidClassException;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_InvalidClassException
#define XMLVM_FORWARD_DECL_java_io_InvalidClassException
typedef struct java_io_InvalidClassException java_io_InvalidClassException;
#endif

#define XMLVM_VTABLE_SIZE_java_io_InvalidClassException 8
#define XMLVM_VTABLE_IDX_java_io_InvalidClassException_getMessage__ 7

void __INIT_java_io_InvalidClassException();
void __INIT_IMPL_java_io_InvalidClassException();
void __DELETE_java_io_InvalidClassException(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_InvalidClassException(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_InvalidClassException();
JAVA_OBJECT __NEW_INSTANCE_java_io_InvalidClassException();
JAVA_LONG java_io_InvalidClassException_GET_serialVersionUID();
void java_io_InvalidClassException_PUT_serialVersionUID(JAVA_LONG v);
void java_io_InvalidClassException___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_InvalidClassException___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 7
JAVA_OBJECT java_io_InvalidClassException_getMessage__(JAVA_OBJECT me);

#endif
