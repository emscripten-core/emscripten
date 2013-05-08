#ifndef __JAVA_NIO_BUFFERFACTORY__
#define __JAVA_NIO_BUFFERFACTORY__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_nio_BufferFactory 0
// Implemented interfaces:
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_lang_CharSequence
#define XMLVM_FORWARD_DECL_java_lang_CharSequence
XMLVM_FORWARD_DECL(java_lang_CharSequence)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ByteBuffer
XMLVM_FORWARD_DECL(java_nio_ByteBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_CharBuffer
#define XMLVM_FORWARD_DECL_java_nio_CharBuffer
XMLVM_FORWARD_DECL(java_nio_CharBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ReadWriteCharArrayBuffer
#define XMLVM_FORWARD_DECL_java_nio_ReadWriteCharArrayBuffer
XMLVM_FORWARD_DECL(java_nio_ReadWriteCharArrayBuffer)
#endif
#ifndef XMLVM_FORWARD_DECL_java_nio_ReadWriteHeapByteBuffer
#define XMLVM_FORWARD_DECL_java_nio_ReadWriteHeapByteBuffer
XMLVM_FORWARD_DECL(java_nio_ReadWriteHeapByteBuffer)
#endif
// Class declarations for java.nio.BufferFactory
XMLVM_DEFINE_CLASS(java_nio_BufferFactory, 6, XMLVM_ITABLE_SIZE_java_nio_BufferFactory)

extern JAVA_OBJECT __CLASS_java_nio_BufferFactory;
extern JAVA_OBJECT __CLASS_java_nio_BufferFactory_1ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_BufferFactory_2ARRAY;
extern JAVA_OBJECT __CLASS_java_nio_BufferFactory_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_nio_BufferFactory
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_nio_BufferFactory \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        __ADDITIONAL_INSTANCE_FIELDS_java_nio_BufferFactory \
    } java_nio_BufferFactory

struct java_nio_BufferFactory {
    __TIB_DEFINITION_java_nio_BufferFactory* tib;
    struct {
        __INSTANCE_FIELDS_java_nio_BufferFactory;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_nio_BufferFactory
#define XMLVM_FORWARD_DECL_java_nio_BufferFactory
typedef struct java_nio_BufferFactory java_nio_BufferFactory;
#endif

#define XMLVM_VTABLE_SIZE_java_nio_BufferFactory 6

void __INIT_java_nio_BufferFactory();
void __INIT_IMPL_java_nio_BufferFactory();
void __DELETE_java_nio_BufferFactory(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_nio_BufferFactory(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_nio_BufferFactory();
JAVA_OBJECT __NEW_INSTANCE_java_nio_BufferFactory();
void java_nio_BufferFactory___INIT___(JAVA_OBJECT me);
JAVA_OBJECT java_nio_BufferFactory_newByteBuffer___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newByteBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newCharBuffer___char_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newCharBuffer___java_lang_CharSequence(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newCharBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newDirectByteBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newDoubleBuffer___double_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newDoubleBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newFloatBuffer___float_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newFloatBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newIntBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newIntBuffer___int_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newLongBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newLongBuffer___long_1ARRAY(JAVA_OBJECT n1);
JAVA_OBJECT java_nio_BufferFactory_newShortBuffer___int(JAVA_INT n1);
JAVA_OBJECT java_nio_BufferFactory_newShortBuffer___short_1ARRAY(JAVA_OBJECT n1);

#endif
