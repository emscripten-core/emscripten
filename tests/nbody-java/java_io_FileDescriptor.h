#ifndef __JAVA_IO_FILEDESCRIPTOR__
#define __JAVA_IO_FILEDESCRIPTOR__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_FileDescriptor 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for java.io.FileDescriptor
XMLVM_DEFINE_CLASS(java_io_FileDescriptor, 6, XMLVM_ITABLE_SIZE_java_io_FileDescriptor)

extern JAVA_OBJECT __CLASS_java_io_FileDescriptor;
extern JAVA_OBJECT __CLASS_java_io_FileDescriptor_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_FileDescriptor_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_FileDescriptor_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_FileDescriptor
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_FileDescriptor \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_LONG descriptor_; \
        JAVA_BOOLEAN readOnly_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_FileDescriptor \
    } java_io_FileDescriptor

struct java_io_FileDescriptor {
    __TIB_DEFINITION_java_io_FileDescriptor* tib;
    struct {
        __INSTANCE_FIELDS_java_io_FileDescriptor;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_FileDescriptor
#define XMLVM_FORWARD_DECL_java_io_FileDescriptor
typedef struct java_io_FileDescriptor java_io_FileDescriptor;
#endif

#define XMLVM_VTABLE_SIZE_java_io_FileDescriptor 6

void __INIT_java_io_FileDescriptor();
void __INIT_IMPL_java_io_FileDescriptor();
void __DELETE_java_io_FileDescriptor(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_FileDescriptor(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_FileDescriptor();
JAVA_OBJECT __NEW_INSTANCE_java_io_FileDescriptor();
void xmlvm_init_native_java_io_FileDescriptor();
JAVA_OBJECT java_io_FileDescriptor_GET_in();
void java_io_FileDescriptor_PUT_in(JAVA_OBJECT v);
JAVA_OBJECT java_io_FileDescriptor_GET_out();
void java_io_FileDescriptor_PUT_out(JAVA_OBJECT v);
JAVA_OBJECT java_io_FileDescriptor_GET_err();
void java_io_FileDescriptor_PUT_err(JAVA_OBJECT v);
void java_io_FileDescriptor___CLINIT_();
void java_io_FileDescriptor_oneTimeInitialization__();
void java_io_FileDescriptor___INIT___(JAVA_OBJECT me);
void java_io_FileDescriptor_sync__(JAVA_OBJECT me);
void java_io_FileDescriptor_syncImpl__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_FileDescriptor_valid__(JAVA_OBJECT me);

#endif
