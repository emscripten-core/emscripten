#ifndef __JAVA_LANG_STACKTRACEELEMENT__
#define __JAVA_LANG_STACKTRACEELEMENT__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_lang_StackTraceElement 0
// Implemented interfaces:
#include "java_io_Serializable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
// Class declarations for java.lang.StackTraceElement
XMLVM_DEFINE_CLASS(java_lang_StackTraceElement, 6, XMLVM_ITABLE_SIZE_java_lang_StackTraceElement)

extern JAVA_OBJECT __CLASS_java_lang_StackTraceElement;
extern JAVA_OBJECT __CLASS_java_lang_StackTraceElement_1ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_StackTraceElement_2ARRAY;
extern JAVA_OBJECT __CLASS_java_lang_StackTraceElement_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_lang_StackTraceElement
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_lang_StackTraceElement \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT declaringClass_; \
        JAVA_OBJECT methodName_; \
        JAVA_OBJECT fileName_; \
        JAVA_INT lineNumber_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_lang_StackTraceElement \
    } java_lang_StackTraceElement

struct java_lang_StackTraceElement {
    __TIB_DEFINITION_java_lang_StackTraceElement* tib;
    struct {
        __INSTANCE_FIELDS_java_lang_StackTraceElement;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_lang_StackTraceElement
#define XMLVM_FORWARD_DECL_java_lang_StackTraceElement
typedef struct java_lang_StackTraceElement java_lang_StackTraceElement;
#endif

#define XMLVM_VTABLE_SIZE_java_lang_StackTraceElement 6
#define XMLVM_VTABLE_IDX_java_lang_StackTraceElement_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_lang_StackTraceElement_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_lang_StackTraceElement_toString__ 5

void __INIT_java_lang_StackTraceElement();
void __INIT_IMPL_java_lang_StackTraceElement();
void __DELETE_java_lang_StackTraceElement(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_lang_StackTraceElement(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_lang_StackTraceElement();
JAVA_OBJECT __NEW_INSTANCE_java_lang_StackTraceElement();
JAVA_LONG java_lang_StackTraceElement_GET_serialVersionUID();
void java_lang_StackTraceElement_PUT_serialVersionUID(JAVA_LONG v);
void java_lang_StackTraceElement___INIT____java_lang_String_java_lang_String_java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_INT n4);
void java_lang_StackTraceElement___INIT___(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_lang_StackTraceElement_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_lang_StackTraceElement_getClassName__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_StackTraceElement_getFileName__(JAVA_OBJECT me);
JAVA_INT java_lang_StackTraceElement_getLineNumber__(JAVA_OBJECT me);
JAVA_OBJECT java_lang_StackTraceElement_getMethodName__(JAVA_OBJECT me);
// Vtable index: 4
JAVA_INT java_lang_StackTraceElement_hashCode__(JAVA_OBJECT me);
JAVA_BOOLEAN java_lang_StackTraceElement_isNativeMethod__(JAVA_OBJECT me);
// Vtable index: 5
JAVA_OBJECT java_lang_StackTraceElement_toString__(JAVA_OBJECT me);

#endif
