#ifndef __JAVA_LANG_SYSTEM__
#define __JAVA_LANG_SYSTEM__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_System 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_PrintStream
#define XMLVM_FORWARD_DECL_java_io_PrintStream
XMLVM_FORWARD_DECL(java_io_PrintStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Boolean
#define XMLVM_FORWARD_DECL_java_lang_Boolean
XMLVM_FORWARD_DECL(java_lang_Boolean)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Byte
#define XMLVM_FORWARD_DECL_java_lang_Byte
XMLVM_FORWARD_DECL(java_lang_Byte)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Double
#define XMLVM_FORWARD_DECL_java_lang_Double
XMLVM_FORWARD_DECL(java_lang_Double)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Float
#define XMLVM_FORWARD_DECL_java_lang_Float
XMLVM_FORWARD_DECL(java_lang_Float)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Integer
#define XMLVM_FORWARD_DECL_java_lang_Integer
XMLVM_FORWARD_DECL(java_lang_Integer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Long
#define XMLVM_FORWARD_DECL_java_lang_Long
XMLVM_FORWARD_DECL(java_lang_Long)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Runtime
#define XMLVM_FORWARD_DECL_java_lang_Runtime
XMLVM_FORWARD_DECL(java_lang_Runtime)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_RuntimePermission
#define XMLVM_FORWARD_DECL_java_lang_RuntimePermission
XMLVM_FORWARD_DECL(java_lang_RuntimePermission)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_SecurityManager
#define XMLVM_FORWARD_DECL_java_lang_SecurityManager
XMLVM_FORWARD_DECL(java_lang_SecurityManager)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Short
#define XMLVM_FORWARD_DECL_java_lang_Short
XMLVM_FORWARD_DECL(java_lang_Short)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Map
#define XMLVM_FORWARD_DECL_java_util_Map
XMLVM_FORWARD_DECL(java_util_Map)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_Properties
#define XMLVM_FORWARD_DECL_java_util_Properties
XMLVM_FORWARD_DECL(java_util_Properties)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_PropertyPermission
#define XMLVM_FORWARD_DECL_java_util_PropertyPermission
XMLVM_FORWARD_DECL(java_util_PropertyPermission)
#endif
#ifndef XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
#define XMLVM_FORWARD_DECL_org_xmlvm_runtime_XMLVMUtil
XMLVM_FORWARD_DECL(org_xmlvm_runtime_XMLVMUtil)
#endif
// Class declarations for java.lang.System
XMLVM_DEFINE_CLASS(java_lang_System, 6, XMLVM_ITABLE_SIZE_java_lang_System)

extern JAVA_OBJECT __CLASS_java_lang_System;
extern JAVA_OBJECT __CLASS_java_lang_System_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_System_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_System_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_System
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_System \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_System \
    } java_lang_System

struct java_lang_System {
    __TIB_DEFINITION_java_lang_System* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_System;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
typedef struct java_lang_System java_lang_System;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_System 6

void __INIT_java_lang_System();
void __INIT_IMPL_java_lang_System();
void __DELETE_java_lang_System(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_System(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_System();
JAVA_OBJECT __NEW_INSTANCE_java_lang_System();
JAVA_OBJECT java_lang_System_GET_in();
void java_lang_System_PUT_in(JAVA_OBJECT v);
JAVA_OBJECT java_lang_System_GET_out();
void java_lang_System_PUT_out(JAVA_OBJECT v);
JAVA_OBJECT java_lang_System_GET_err();
void java_lang_System_PUT_err(JAVA_OBJECT v);
JAVA_OBJECT java_lang_System_GET_RUNTIME();
void java_lang_System_PUT_RUNTIME(JAVA_OBJECT v);
JAVA_OBJECT java_lang_System_GET_systemProperties();
void java_lang_System_PUT_systemProperties(JAVA_OBJECT v);
JAVA_OBJECT java_lang_System_GET_security();
void java_lang_System_PUT_security(JAVA_OBJECT v);
JAVA_INT java_lang_System_GET_InitLocale();
void java_lang_System_PUT_InitLocale(JAVA_INT v);
JAVA_INT java_lang_System_GET_PlatformEncoding();
void java_lang_System_PUT_PlatformEncoding(JAVA_INT v);
JAVA_INT java_lang_System_GET_FileEncoding();
void java_lang_System_PUT_FileEncoding(JAVA_INT v);
JAVA_INT java_lang_System_GET_OSEncoding();
void java_lang_System_PUT_OSEncoding(JAVA_INT v);
void java_lang_System_initNativeLayer__();
void java_lang_System_setIn___java_io_InputStream(JAVA_OBJECT n1);
void java_lang_System_setOut___java_io_PrintStream(JAVA_OBJECT n1);
void java_lang_System_setErr___java_io_PrintStream(JAVA_OBJECT n1);
void java_lang_System___INIT___(JAVA_OBJECT me);
void java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___java_lang_Object_1ARRAY_int_java_lang_Object_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___int_1ARRAY_int_int_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___byte_1ARRAY_int_byte_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___short_1ARRAY_int_short_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___long_1ARRAY_int_long_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___char_1ARRAY_int_char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___boolean_1ARRAY_int_boolean_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___double_1ARRAY_int_double_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
void java_lang_System_arraycopy___float_1ARRAY_int_float_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4, JAVA_INT n5);
JAVA_LONG java_lang_System_currentTimeMillis__();
JAVA_LONG java_lang_System_nanoTime__();
void java_lang_System_ensureProperties__();
void java_lang_System_exit___int(JAVA_INT n1);
void java_lang_System_nativeExit___int(JAVA_INT n1);
void java_lang_System_gc__();
JAVA_OBJECT java_lang_System_getenv___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_System_getenv__();
JAVA_OBJECT java_lang_System_inheritedChannel__();
JAVA_OBJECT java_lang_System_getProperties__();
JAVA_OBJECT java_lang_System_internalGetProperties__();
JAVA_OBJECT java_lang_System_getProperty___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_System_getProperty___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_System_setProperty___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_System_clearProperty___java_lang_String(JAVA_OBJECT n1);
JAVA_OBJECT java_lang_System_getPropertyList__();
JAVA_OBJECT java_lang_System_getEncoding___int(JAVA_INT n1);
JAVA_OBJECT java_lang_System_getSecurityManager__();
JAVA_INT java_lang_System_identityHashCode___java_lang_Object(JAVA_OBJECT n1);
void java_lang_System_loadLibrary___java_lang_String(JAVA_OBJECT n1);
void java_lang_System_runFinalization__();
void java_lang_System_runFinalizersOnExit___boolean(JAVA_BOOLEAN n1);
void java_lang_System_setProperties___java_util_Properties(JAVA_OBJECT n1);
void java_lang_System_setSecurityManager___java_lang_SecurityManager(JAVA_OBJECT n1);
void java_lang_System___CLINIT_();

#endif
