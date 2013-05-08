#ifndef __JAVA_IO_PRINTSTREAM__
#define __JAVA_IO_PRINTSTREAM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_PrintStream 5
// Implemented interfaces:
#include "java_io_Closeable.h"
#include "java_lang_Appendable.h"
// Super Class:
#include "java_io_FilterOutputStream.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_File
#define XMLVM_FORWARD_DECL_java_io_File
XMLVM_FORWARD_DECL(java_io_File)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
XMLVM_FORWARD_DECL(java_io_IOException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_OutputStream
#define XMLVM_FORWARD_DECL_java_io_OutputStream
XMLVM_FORWARD_DECL(java_io_OutputStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
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
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Locale
#define XMLVM_FORWARD_DECL_java_util_Locale
XMLVM_FORWARD_DECL(java_util_Locale)
#endif
// Class declarations for java.io.PrintStream
XMLVM_DEFINE_CLASS(java_io_PrintStream, 15, XMLVM_ITABLE_SIZE_java_io_PrintStream)

extern JAVA_OBJECT __CLASS_java_io_PrintStream;
extern JAVA_OBJECT __CLASS_java_io_PrintStream_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_PrintStream_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_PrintStream_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_PrintStream
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_PrintStream \
    __INSTANCE_FIELDS_java_io_FilterOutputStream; \
    struct { \
        JAVA_BOOLEAN ioError_; \
        JAVA_BOOLEAN autoflush_; \
        JAVA_OBJECT encoding_; \
        JAVA_OBJECT lineSeparator_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_PrintStream \
    } java_io_PrintStream

struct java_io_PrintStream {
    __TIB_DEFINITION_java_io_PrintStream* tib;
    struct {
        __INSTANCE_FIELDS_java_io_PrintStream;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
typedef struct java_io_PrintStream java_io_PrintStream;
#endif

#define XMLVM_VTABLE_SIZE_java_io_PrintStream 15
#define XMLVM_VTABLE_IDX_java_io_PrintStream_checkError__ 6
#define XMLVM_VTABLE_IDX_java_io_PrintStream_close__ 7
#define XMLVM_VTABLE_IDX_java_io_PrintStream_flush__ 8
#define XMLVM_VTABLE_IDX_java_io_PrintStream_write___byte_1ARRAY_int_int 10
#define XMLVM_VTABLE_IDX_java_io_PrintStream_write___int 11
#define XMLVM_VTABLE_IDX_java_io_PrintStream_append___char 12
#define XMLVM_VTABLE_IDX_java_io_PrintStream_append___java_lang_CharSequence 13
#define XMLVM_VTABLE_IDX_java_io_PrintStream_append___java_lang_CharSequence_int_int 14

void __INIT_java_io_PrintStream();
void __INIT_IMPL_java_io_PrintStream();
void __DELETE_java_io_PrintStream(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_PrintStream(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_PrintStream();
JAVA_OBJECT __NEW_INSTANCE_java_io_PrintStream();
JAVA_OBJECT java_io_PrintStream_GET_TOKEN_NULL();
void java_io_PrintStream_PUT_TOKEN_NULL(JAVA_OBJECT v);
void java_io_PrintStream___INIT____java_io_OutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream___INIT____java_io_OutputStream_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_io_PrintStream___INIT____java_io_OutputStream_boolean_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_OBJECT n3);
void java_io_PrintStream___INIT____java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream___INIT____java_io_File_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_PrintStream___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 6
JAVA_BOOLEAN java_io_PrintStream_checkError__(JAVA_OBJECT me);
// Vtable index: 7
void java_io_PrintStream_close__(JAVA_OBJECT me);
// Vtable index: 8
void java_io_PrintStream_flush__(JAVA_OBJECT me);
JAVA_OBJECT java_io_PrintStream_format___java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_io_PrintStream_format___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_io_PrintStream_printf___java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_io_PrintStream_printf___java_util_Locale_java_lang_String_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_io_PrintStream_newline__(JAVA_OBJECT me);
void java_io_PrintStream_print___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_print___char(JAVA_OBJECT me, JAVA_CHAR n1);
void java_io_PrintStream_print___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
void java_io_PrintStream_print___float(JAVA_OBJECT me, JAVA_FLOAT n1);
void java_io_PrintStream_print___int(JAVA_OBJECT me, JAVA_INT n1);
void java_io_PrintStream_print___long(JAVA_OBJECT me, JAVA_LONG n1);
void java_io_PrintStream_print___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_print___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_print___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_io_PrintStream_println__(JAVA_OBJECT me);
void java_io_PrintStream_println___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_println___char(JAVA_OBJECT me, JAVA_CHAR n1);
void java_io_PrintStream_println___double(JAVA_OBJECT me, JAVA_DOUBLE n1);
void java_io_PrintStream_println___float(JAVA_OBJECT me, JAVA_FLOAT n1);
void java_io_PrintStream_println___int(JAVA_OBJECT me, JAVA_INT n1);
void java_io_PrintStream_println___long(JAVA_OBJECT me, JAVA_LONG n1);
void java_io_PrintStream_println___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_println___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_PrintStream_println___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_io_PrintStream_setError__(JAVA_OBJECT me);
// Vtable index: 10
void java_io_PrintStream_write___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);
// Vtable index: 11
void java_io_PrintStream_write___int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 12
JAVA_OBJECT java_io_PrintStream_append___char(JAVA_OBJECT me, JAVA_CHAR n1);
// Vtable index: 13
JAVA_OBJECT java_io_PrintStream_append___java_lang_CharSequence(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 14
JAVA_OBJECT java_io_PrintStream_append___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3);

#endif
