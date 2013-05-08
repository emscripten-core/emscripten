#ifndef __JAVA_LANG_RUNTIME__
#define __JAVA_LANG_RUNTIME__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_Runtime 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_File
#define XMLVM_FORWARD_DECL_java_io_File
XMLVM_FORWARD_DECL(java_io_File)
#endif
#ifndef XMLVM_FORWARD_DECL_java_io_OutputStream
#define XMLVM_FORWARD_DECL_java_io_OutputStream
XMLVM_FORWARD_DECL(java_io_OutputStream)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Thread
#define XMLVM_FORWARD_DECL_java_lang_Thread
XMLVM_FORWARD_DECL(java_lang_Thread)
#endif
// Class declarations for java.lang.Runtime
XMLVM_DEFINE_CLASS(java_lang_Runtime, 6, XMLVM_ITABLE_SIZE_java_lang_Runtime)

extern JAVA_OBJECT __CLASS_java_lang_Runtime;
extern JAVA_OBJECT __CLASS_java_lang_Runtime_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Runtime_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_Runtime_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_Runtime
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_Runtime \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_Runtime \
    } java_lang_Runtime

struct java_lang_Runtime {
    __TIB_DEFINITION_java_lang_Runtime* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_Runtime;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_Runtime
#define XMLVM_FORWARD_DECL_java_lang_Runtime
typedef struct java_lang_Runtime java_lang_Runtime;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_Runtime 6

void __INIT_java_lang_Runtime();
void __INIT_IMPL_java_lang_Runtime();
void __DELETE_java_lang_Runtime(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_Runtime(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_Runtime();
JAVA_OBJECT __NEW_INSTANCE_java_lang_Runtime();
void xmlvm_init_native_java_lang_Runtime();
JAVA_OBJECT java_lang_Runtime_GET_runtime();
void java_lang_Runtime_PUT_runtime(JAVA_OBJECT v);
void java_lang_Runtime___INIT___(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_1ARRAY_java_lang_String_1ARRAY_java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_lang_Runtime_exec___java_lang_String_java_lang_String_1ARRAY_java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
void java_lang_Runtime_exit___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_LONG java_lang_Runtime_freeMemory__(JAVA_OBJECT me);
void java_lang_Runtime_gc__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_Runtime_getRuntime__();
void java_lang_Runtime_load___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Runtime_loadLibrary___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Runtime_runFinalization__(JAVA_OBJECT me);
void java_lang_Runtime_runFinalizersOnExit___boolean(JAVA_BOOLEAN n1);
JAVA_LONG java_lang_Runtime_totalMemory__(JAVA_OBJECT me);
void java_lang_Runtime_traceInstructions___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
void java_lang_Runtime_traceMethodCalls___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_OBJECT java_lang_Runtime_getLocalizedInputStream___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_Runtime_getLocalizedOutputStream___java_io_OutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Runtime_addShutdownHook___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_lang_Runtime_removeShutdownHook___java_lang_Thread(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_lang_Runtime_halt___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_lang_Runtime_availableProcessors__(JAVA_OBJECT me);
JAVA_LONG java_lang_Runtime_maxMemory__(JAVA_OBJECT me);
void java_lang_Runtime___CLINIT_();

#endif
