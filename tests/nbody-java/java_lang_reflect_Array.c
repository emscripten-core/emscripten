#include "xmlvm.h"
#include "java_lang_Class.h"

#include "java_lang_reflect_Array.h"

#define XMLVM_CURRENT_CLASS_NAME Array
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_reflect_Array

__TIB_DEFINITION_java_lang_reflect_Array __TIB_java_lang_reflect_Array = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_Array, // classInitializer
    "java.lang.reflect.Array", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_reflect_Array), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_reflect_Array;
JAVA_OBJECT __CLASS_java_lang_reflect_Array_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Array_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Array_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION


#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_java_lang_reflect_Array()
{
    staticInitializerLock(&__TIB_java_lang_reflect_Array);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_Array.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_Array.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_Array);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_Array.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_Array.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_Array.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.Array")
        __INIT_IMPL_java_lang_reflect_Array();
    }
}

void __INIT_IMPL_java_lang_reflect_Array()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_reflect_Array.newInstanceFunc = __NEW_INSTANCE_java_lang_reflect_Array;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_reflect_Array.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_lang_reflect_Array.numImplementedInterfaces = 0;
    __TIB_java_lang_reflect_Array.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_java_lang_reflect_Array.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_Array.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_reflect_Array.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_reflect_Array.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_reflect_Array.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_reflect_Array.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_Array.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_Array.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_reflect_Array = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_Array);
    __TIB_java_lang_reflect_Array.clazz = __CLASS_java_lang_reflect_Array;
    __TIB_java_lang_reflect_Array.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_Array_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Array);
    __CLASS_java_lang_reflect_Array_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Array_1ARRAY);
    __CLASS_java_lang_reflect_Array_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Array_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_reflect_Array]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_reflect_Array.classInitialized = 1;
}

void __DELETE_java_lang_reflect_Array(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_reflect_Array]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Array(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_reflect_Array]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_reflect_Array()
{    XMLVM_CLASS_INIT(java_lang_reflect_Array)
java_lang_reflect_Array* me = (java_lang_reflect_Array*) XMLVM_MALLOC(sizeof(java_lang_reflect_Array));
    me->tib = &__TIB_java_lang_reflect_Array;
    __INIT_INSTANCE_MEMBERS_java_lang_reflect_Array(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_reflect_Array]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Array()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_reflect_Array___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Array___INIT___]
    XMLVM_ENTER_METHOD("java.lang.reflect.Array", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Array.java", 28)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Array.java", 30)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Array_multiNewArrayImpl___java_lang_Class_int_int_1ARRAY(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Array_newArrayImpl___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Array_get___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_Array_getBoolean___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_BYTE java_lang_reflect_Array_getByte___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_CHAR java_lang_reflect_Array_getChar___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_DOUBLE java_lang_reflect_Array_getDouble___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_FLOAT java_lang_reflect_Array_getFloat___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_INT java_lang_reflect_Array_getInt___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_INT java_lang_reflect_Array_getLength___java_lang_Object(JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_LONG java_lang_reflect_Array_getLong___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_SHORT java_lang_reflect_Array_getShort___java_lang_Object_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Array_newInstance___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[void java_lang_reflect_Array_set___java_lang_Object_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setBoolean___java_lang_Object_int_boolean(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BOOLEAN n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setByte___java_lang_Object_int_byte(JAVA_OBJECT n1, JAVA_INT n2, JAVA_BYTE n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setChar___java_lang_Object_int_char(JAVA_OBJECT n1, JAVA_INT n2, JAVA_CHAR n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setDouble___java_lang_Object_int_double(JAVA_OBJECT n1, JAVA_INT n2, JAVA_DOUBLE n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setFloat___java_lang_Object_int_float(JAVA_OBJECT n1, JAVA_INT n2, JAVA_FLOAT n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setInt___java_lang_Object_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setLong___java_lang_Object_int_long(JAVA_OBJECT n1, JAVA_INT n2, JAVA_LONG n3)]

//XMLVM_NATIVE[void java_lang_reflect_Array_setShort___java_lang_Object_int_short(JAVA_OBJECT n1, JAVA_INT n2, JAVA_SHORT n3)]

