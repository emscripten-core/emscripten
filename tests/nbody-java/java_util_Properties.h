#ifndef __JAVA_UTIL_PROPERTIES__
#define __JAVA_UTIL_PROPERTIES__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_util_Properties 14
// Implemented interfaces:
// Super Class:
#include "java_util_Hashtable.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
XMLVM_FORWARD_DECL(java_io_IOException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_OutputStream
#define XMLVM_FORWARD_DECL_java_io_OutputStream
XMLVM_FORWARD_DECL(java_io_OutputStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
XMLVM_FORWARD_DECL(java_io_PrintStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
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
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Throwable
#define XMLVM_FORWARD_DECL_java_lang_Throwable
XMLVM_FORWARD_DECL(java_lang_Throwable)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_charset_Charset
#define XMLVM_FORWARD_DECL_java_nio_charset_Charset
XMLVM_FORWARD_DECL(java_nio_charset_Charset)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_AccessController
#define XMLVM_FORWARD_DECL_java_security_AccessController
XMLVM_FORWARD_DECL(java_security_AccessController)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Collections
#define XMLVM_FORWARD_DECL_java_util_Collections
XMLVM_FORWARD_DECL(java_util_Collections)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Enumeration
#define XMLVM_FORWARD_DECL_java_util_Enumeration
XMLVM_FORWARD_DECL(java_util_Enumeration)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Iterator
#define XMLVM_FORWARD_DECL_java_util_Iterator
XMLVM_FORWARD_DECL(java_util_Iterator)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Map_Entry
#define XMLVM_FORWARD_DECL_java_util_Map_Entry
XMLVM_FORWARD_DECL(java_util_Map_Entry)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Set
#define XMLVM_FORWARD_DECL_java_util_Set
XMLVM_FORWARD_DECL(java_util_Set)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_PriviAction)
#endif
// Class declarations for java.util.Properties
XMLVM_DEFINE_CLASS(java_util_Properties, 20, XMLVM_ITABLE_SIZE_java_util_Properties)

extern JAVA_OBJECT __CLASS_java_util_Properties;
extern JAVA_OBJECT __CLASS_java_util_Properties_1ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Properties_2ARRAY;
extern JAVA_OBJECT __CLASS_java_util_Properties_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_util_Properties
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_util_Properties \
    __INSTANCE_FIELDS_java_util_Hashtable; \
    struct { \
        JAVA_OBJECT builder_; \
        JAVA_OBJECT defaults_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_util_Properties \
    } java_util_Properties

struct java_util_Properties {
    __TIB_DEFINITION_java_util_Properties* tib;
    struct {
        __INSTANCE_FIELDS_java_util_Properties;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_util_Properties
#define XMLVM_FORWARD_DECL_java_util_Properties
typedef struct java_util_Properties java_util_Properties;
#endif

#define XMLVM_VTABLE_SIZE_java_util_Properties 20

void __INIT_java_util_Properties();
void __INIT_IMPL_java_util_Properties();
void __DELETE_java_util_Properties(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_util_Properties(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_util_Properties();
JAVA_OBJECT __NEW_INSTANCE_java_util_Properties();
JAVA_LONG java_util_Properties_GET_serialVersionUID();
void java_util_Properties_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_util_Properties_GET_PROP_DTD_NAME();
void java_util_Properties_PUT_PROP_DTD_NAME(JAVA_OBJECT v);
JAVA_OBJECT java_util_Properties_GET_PROP_DTD();
void java_util_Properties_PUT_PROP_DTD(JAVA_OBJECT v);
JAVA_INT java_util_Properties_GET_NONE();
void java_util_Properties_PUT_NONE(JAVA_INT v);
JAVA_INT java_util_Properties_GET_SLASH();
void java_util_Properties_PUT_SLASH(JAVA_INT v);
JAVA_INT java_util_Properties_GET_UNICODE();
void java_util_Properties_PUT_UNICODE(JAVA_INT v);
JAVA_INT java_util_Properties_GET_CONTINUE();
void java_util_Properties_PUT_CONTINUE(JAVA_INT v);
JAVA_INT java_util_Properties_GET_KEY_DONE();
void java_util_Properties_PUT_KEY_DONE(JAVA_INT v);
JAVA_INT java_util_Properties_GET_IGNORE();
void java_util_Properties_PUT_IGNORE(JAVA_INT v);
JAVA_OBJECT java_util_Properties_GET_lineSeparator();
void java_util_Properties_PUT_lineSeparator(JAVA_OBJECT v);
void java_util_Properties___INIT___(JAVA_OBJECT me);
void java_util_Properties___INIT____java_util_Properties(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4);
JAVA_OBJECT java_util_Properties_toHexaDecimal___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_util_Properties_getProperty___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Properties_getProperty___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_list___java_io_PrintStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_list___java_io_PrintWriter(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_load___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_util_Properties_isEbcdic___java_io_BufferedInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_load___java_io_Reader(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_loadImpl___java_io_Reader(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_util_Properties_propertyNames__(JAVA_OBJECT me);
JAVA_OBJECT java_util_Properties_stringPropertyNames__(JAVA_OBJECT me);
void java_util_Properties_selectProperties___java_util_Hashtable_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2);
void java_util_Properties_save___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_util_Properties_setProperty___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_store___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_store___java_io_Writer_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_writeComments___java_io_Writer_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_loadFromXML___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_util_Properties_substitutePredefinedEntries___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);

#endif
