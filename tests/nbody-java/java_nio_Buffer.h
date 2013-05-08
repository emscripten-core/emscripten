#ifndef __JAVA_NIO_BUFFER__
#define __JAVA_NIO_BUFFER__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_Buffer 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
// Class declarations for java.nio.Buffer
XMLVM_DEFINE_CLASS(java_nio_Buffer, 11, XMLVM_ITABLE_SIZE_java_nio_Buffer)

extern JAVA_OBJECT __CLASS_java_nio_Buffer;
extern JAVA_OBJECT __CLASS_java_nio_Buffer_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_Buffer_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_Buffer_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_Buffer
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_Buffer \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_INT capacity_; \
        JAVA_INT limit_; \
        JAVA_INT mark_; \
        JAVA_INT position_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_Buffer \
    } java_nio_Buffer

struct java_nio_Buffer {
    __TIB_DEFINITION_java_nio_Buffer* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_Buffer;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_Buffer
#define XMLVM_FORWARD_DECL_java_nio_Buffer
typedef struct java_nio_Buffer java_nio_Buffer;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_Buffer 11
#define XMLVM_VTABLE_IDX_java_nio_Buffer_array__ 7
#define XMLVM_VTABLE_IDX_java_nio_Buffer_arrayOffset__ 6
#define XMLVM_VTABLE_IDX_java_nio_Buffer_hasArray__ 8
#define XMLVM_VTABLE_IDX_java_nio_Buffer_isDirect__ 9
#define XMLVM_VTABLE_IDX_java_nio_Buffer_isReadOnly__ 10

void __INIT_java_nio_Buffer();
void __INIT_IMPL_java_nio_Buffer();
void __DELETE_java_nio_Buffer(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_Buffer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_Buffer();
JAVA_OBJECT __NEW_INSTANCE_java_nio_Buffer();
JAVA_INT java_nio_Buffer_GET_UNSET_MARK();
void java_nio_Buffer_PUT_UNSET_MARK(JAVA_INT v);
void java_nio_Buffer___INIT____int(JAVA_OBJECT me, JAVA_INT n1);
// Vtable index: 7
JAVA_OBJECT java_nio_Buffer_array__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_INT java_nio_Buffer_arrayOffset__(JAVA_OBJECT me);
JAVA_INT java_nio_Buffer_capacity__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_clear__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_flip__(JAVA_OBJECT me);
// Vtable index: 8
JAVA_BOOLEAN java_nio_Buffer_hasArray__(JAVA_OBJECT me);
JAVA_BOOLEAN java_nio_Buffer_hasRemaining__(JAVA_OBJECT me);
// Vtable index: 9
JAVA_BOOLEAN java_nio_Buffer_isDirect__(JAVA_OBJECT me);
// Vtable index: 10
JAVA_BOOLEAN java_nio_Buffer_isReadOnly__(JAVA_OBJECT me);
JAVA_INT java_nio_Buffer_limit__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_limit___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_OBJECT java_nio_Buffer_mark__(JAVA_OBJECT me);
JAVA_INT java_nio_Buffer_position__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_position___int(JAVA_OBJECT me, JAVA_INT n1);
JAVA_INT java_nio_Buffer_remaining__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_reset__(JAVA_OBJECT me);
JAVA_OBJECT java_nio_Buffer_rewind__(JAVA_OBJECT me);

#endif
