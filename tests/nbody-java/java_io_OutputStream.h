#ifndef __JAVA_IO_OUTPUTSTREAM__
#define __JAVA_IO_OUTPUTSTREAM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_OutputStream 2
// Implemented interfaces:
#include "java_io_Closeable.h"
#include "java_io_Flushable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
#define XMLVM_FORWARD_DECL_java_lang_IndexOutOfBoundsException
XMLVM_FORWARD_DECL(java_lang_IndexOutOfBoundsException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.io.OutputStream
XMLVM_DEFINE_CLASS(java_io_OutputStream, 12, XMLVM_ITABLE_SIZE_java_io_OutputStream)

extern JAVA_OBJECT __CLASS_java_io_OutputStream;
extern JAVA_OBJECT __CLASS_java_io_OutputStream_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_OutputStream_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_OutputStream_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_OutputStream
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_OutputStream \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_OutputStream \
    } java_io_OutputStream

struct java_io_OutputStream {
    __TIB_DEFINITION_java_io_OutputStream* tib;
    struct {
        __INSTANCE_FIELDS_java_io_OutputStream;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_OutputStream
#define XMLVM_FORWARD_DECL_java_io_OutputStream
typedef struct java_io_OutputStream java_io_OutputStream;
#endif

#define XMLVM_VTABLE_SIZE_java_io_OutputStream 12
#define XMLVM_VTABLE_IDX_java_io_OutputStream_close__ 7
#define XMLVM_VTABLE_IDX_java_io_OutputStream_flush__ 8
#define XMLVM_VTABLE_IDX_java_io_OutputStream_write___byte_1ARRAY 9
#define XMLVM_VTABLE_IDX_java_io_OutputStream_write___byte_1ARRAY_int_int 10
#define XMLVM_VTABLE_IDX_java_io_OutputStream_write___int 11
#define XMLVM_VTABLE_IDX_java_io_OutputStream_checkError__ 6

void __INIT_java_io_OutputStream();
void __INIT_IMPL_java_io_OutputStream();
void __DELETE_java_io_OutputStream(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_OutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_OutputStream();
JAVA_OBJECT __NEW_INSTANCE_java_io_OutputStream();
void java_io_OutputStream___INIT___(JAVA_OBJECT me);
// Vtable index: 7
void java_io_OutputStream_close__(JAVA_OBJECT me);
// Vtable index: 8
void java_io_OutputStream_flush__(JAVA_OBJECT me);
// Vtable index: 9
void java_io_OutputStream_write___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 10
void java_io_OutputStream_write___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 11
void java_io_OutputStream_write___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 6
JAVA_BOOLEAN java_io_OutputStream_checkError__(JAVA_OBJECT me);

#endif
