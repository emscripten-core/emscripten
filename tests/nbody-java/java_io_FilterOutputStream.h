#ifndef __JAVA_IO_FILTEROUTPUTSTREAM__
#define __JAVA_IO_FILTEROUTPUTSTREAM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_FilterOutputStream 2
// Implemented interfaces:
// Super Class:
#include "java_io_OutputStream.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
// Class declarations for java.io.FilterOutputStream
XMLVM_DEFINE_CLASS(java_io_FilterOutputStream, 12, XMLVM_ITABLE_SIZE_java_io_FilterOutputStream)

extern JAVA_OBJECT __CLASS_java_io_FilterOutputStream;
extern JAVA_OBJECT __CLASS_java_io_FilterOutputStream_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_FilterOutputStream_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_FilterOutputStream_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_FilterOutputStream
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_FilterOutputStream \
    __INSTANCE_FIELDS_java_io_OutputStream; \
    struct { \
        JAVA_OBJECT out_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_FilterOutputStream \
    } java_io_FilterOutputStream

struct java_io_FilterOutputStream {
    __TIB_DEFINITION_java_io_FilterOutputStream* tib;
    struct {
        __INSTANCE_FIELDS_java_io_FilterOutputStream;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_FilterOutputStream
#define XMLVM_FORWARD_DECL_java_io_FilterOutputStream
typedef struct java_io_FilterOutputStream java_io_FilterOutputStream;
#endif

#define XMLVM_VTABLE_SIZE_java_io_FilterOutputStream 12
#define XMLVM_VTABLE_IDX_java_io_FilterOutputStream_close__ 7
#define XMLVM_VTABLE_IDX_java_io_FilterOutputStream_flush__ 8
#define XMLVM_VTABLE_IDX_java_io_FilterOutputStream_write___byte_1ARRAY 9
#define XMLVM_VTABLE_IDX_java_io_FilterOutputStream_write___byte_1ARRAY_int_int 10
#define XMLVM_VTABLE_IDX_java_io_FilterOutputStream_write___int 11

void __INIT_java_io_FilterOutputStream();
void __INIT_IMPL_java_io_FilterOutputStream();
void __DELETE_java_io_FilterOutputStream(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_FilterOutputStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_FilterOutputStream();
JAVA_OBJECT __NEW_INSTANCE_java_io_FilterOutputStream();
void java_io_FilterOutputStream___INIT____java_io_OutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 7
void java_io_FilterOutputStream_close__(JAVA_OBJECT me);
// Vtable index: 8
void java_io_FilterOutputStream_flush__(JAVA_OBJECT me);
// Vtable index: 9
void java_io_FilterOutputStream_write___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 10
void java_io_FilterOutputStream_write___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 11
void java_io_FilterOutputStream_write___int(JAVA_OBJECT me, JAVA_INT n1);

#endif
