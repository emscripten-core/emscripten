#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_String.h"

#include "java_lang_reflect_AccessibleObject.h"

#define XMLVM_CURRENT_CLASS_NAME AccessibleObject
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_reflect_AccessibleObject

__TIB_DEFINITION_java_lang_reflect_AccessibleObject __TIB_java_lang_reflect_AccessibleObject = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_AccessibleObject, // classInitializer
    "java.lang.reflect.AccessibleObject", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_reflect_AccessibleObject), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_reflect_AccessibleObject;
JAVA_OBJECT __CLASS_java_lang_reflect_AccessibleObject_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_AccessibleObject_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_AccessibleObject_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_lang_reflect_AccessibleObject_emptyArgs;

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

void __INIT_java_lang_reflect_AccessibleObject()
{
    staticInitializerLock(&__TIB_java_lang_reflect_AccessibleObject);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_AccessibleObject.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_AccessibleObject.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_AccessibleObject);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_AccessibleObject.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_AccessibleObject.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_AccessibleObject.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.AccessibleObject")
        __INIT_IMPL_java_lang_reflect_AccessibleObject();
    }
}

void __INIT_IMPL_java_lang_reflect_AccessibleObject()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_reflect_AccessibleObject.newInstanceFunc = __NEW_INSTANCE_java_lang_reflect_AccessibleObject;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_reflect_AccessibleObject.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_reflect_AccessibleObject.vtable[9] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_AccessibleObject.vtable[10] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_AccessibleObject.vtable[11] = (VTABLE_PTR) &java_lang_reflect_AccessibleObject_isAnnotationPresent___java_lang_Class;
    __TIB_java_lang_reflect_AccessibleObject.vtable[8] = (VTABLE_PTR) &java_lang_reflect_AccessibleObject_getDeclaredAnnotations__;
    __TIB_java_lang_reflect_AccessibleObject.vtable[7] = (VTABLE_PTR) &java_lang_reflect_AccessibleObject_getAnnotations__;
    __TIB_java_lang_reflect_AccessibleObject.vtable[6] = (VTABLE_PTR) &java_lang_reflect_AccessibleObject_getAnnotation___java_lang_Class;
    xmlvm_init_native_java_lang_reflect_AccessibleObject();
    // Initialize interface information
    __TIB_java_lang_reflect_AccessibleObject.numImplementedInterfaces = 1;
    __TIB_java_lang_reflect_AccessibleObject.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_reflect_AnnotatedElement)

    __TIB_java_lang_reflect_AccessibleObject.implementedInterfaces[0][0] = &__TIB_java_lang_reflect_AnnotatedElement;
    // Initialize itable for this class
    __TIB_java_lang_reflect_AccessibleObject.itableBegin = &__TIB_java_lang_reflect_AccessibleObject.itable[0];
    __TIB_java_lang_reflect_AccessibleObject.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotation___java_lang_Class] = __TIB_java_lang_reflect_AccessibleObject.vtable[6];
    __TIB_java_lang_reflect_AccessibleObject.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotations__] = __TIB_java_lang_reflect_AccessibleObject.vtable[7];
    __TIB_java_lang_reflect_AccessibleObject.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getDeclaredAnnotations__] = __TIB_java_lang_reflect_AccessibleObject.vtable[8];
    __TIB_java_lang_reflect_AccessibleObject.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_isAnnotationPresent___java_lang_Class] = __TIB_java_lang_reflect_AccessibleObject.vtable[11];

    _STATIC_java_lang_reflect_AccessibleObject_emptyArgs = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;

    __TIB_java_lang_reflect_AccessibleObject.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_AccessibleObject.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_reflect_AccessibleObject.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_reflect_AccessibleObject.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_reflect_AccessibleObject.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_reflect_AccessibleObject.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_AccessibleObject.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_AccessibleObject.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_reflect_AccessibleObject = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_AccessibleObject);
    __TIB_java_lang_reflect_AccessibleObject.clazz = __CLASS_java_lang_reflect_AccessibleObject;
    __TIB_java_lang_reflect_AccessibleObject.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_AccessibleObject_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AccessibleObject);
    __CLASS_java_lang_reflect_AccessibleObject_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AccessibleObject_1ARRAY);
    __CLASS_java_lang_reflect_AccessibleObject_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_AccessibleObject_2ARRAY);
    java_lang_reflect_AccessibleObject___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_reflect_AccessibleObject]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_reflect_AccessibleObject.classInitialized = 1;
}

void __DELETE_java_lang_reflect_AccessibleObject(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_reflect_AccessibleObject]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_reflect_AccessibleObject(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_reflect_AccessibleObject]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_reflect_AccessibleObject()
{    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
java_lang_reflect_AccessibleObject* me = (java_lang_reflect_AccessibleObject*) XMLVM_MALLOC(sizeof(java_lang_reflect_AccessibleObject));
    me->tib = &__TIB_java_lang_reflect_AccessibleObject;
    __INIT_INSTANCE_MEMBERS_java_lang_reflect_AccessibleObject(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_reflect_AccessibleObject]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_AccessibleObject()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_reflect_AccessibleObject();
    java_lang_reflect_AccessibleObject___INIT___(me);
    return me;
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_GET_emptyArgs()
{
    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
    return _STATIC_java_lang_reflect_AccessibleObject_emptyArgs;
}

void java_lang_reflect_AccessibleObject_PUT_emptyArgs(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
_STATIC_java_lang_reflect_AccessibleObject_emptyArgs = v;
}

void java_lang_reflect_AccessibleObject___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 48)
    _r0.i = 0;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r0.i);
    java_lang_reflect_AccessibleObject_PUT_emptyArgs( _r0.o);
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 39)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_reflect_AccessibleObject_setAccessible___java_lang_reflect_AccessibleObject_1ARRAY_boolean(JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_setAccessible___java_lang_reflect_AccessibleObject_1ARRAY_boolean]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "setAccessible", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = n1;
    _r1.i = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 68)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_marshallArguments___java_lang_Class_1ARRAY_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_marshallArguments___java_lang_Class_1ARRAY_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "marshallArguments", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 83)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void java_lang_reflect_AccessibleObject_initializeClass___java_lang_Class(JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_AccessibleObject_getStackClass___int(JAVA_INT n1)]

void java_lang_reflect_AccessibleObject___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject___INIT___]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 108)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 109)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_AccessibleObject_getParameterTypesImpl__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_INT java_lang_reflect_AccessibleObject_getModifiers__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_AccessibleObject_getExceptionTypesImpl__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_AccessibleObject_getSignature__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_AccessibleObject_checkAccessibility___java_lang_Class_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)]

JAVA_BOOLEAN java_lang_reflect_AccessibleObject_isAccessible__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_isAccessible__]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "isAccessible", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 166)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_reflect_AccessibleObject_setAccessible___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_setAccessible___boolean]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "setAccessible", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 180)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_reflect_AccessibleObject_isAnnotationPresent___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_isAnnotationPresent___java_lang_Class]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "isAnnotationPresent", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 184)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_getDeclaredAnnotations__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_getDeclaredAnnotations__]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "getDeclaredAnnotations", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 188)
    _r0.i = 0;

    
    // Red class access removed: java.lang.annotation.Annotation[]::new-array
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_getAnnotations__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_getAnnotations__]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "getAnnotations", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 192)
    _r0.i = 0;

    
    // Red class access removed: java.lang.annotation.Annotation[]::new-array
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_getAnnotation___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_getAnnotation___java_lang_Class]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "getAnnotation", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 196)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_reflect_AccessibleObject_invokeV___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeV___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeV", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 209)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_AccessibleObject_invokeL___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeL___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeL", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 223)
    _r0.o = JAVA_NULL;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_reflect_AccessibleObject_invokeI___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeI___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeI", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 237)
    _r0.i = 0;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG java_lang_reflect_AccessibleObject_invokeJ___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeJ___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeJ", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 251)
    _r0.l = 0;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

JAVA_FLOAT java_lang_reflect_AccessibleObject_invokeF___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeF___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeF", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 265)
    _r0.f = 0.0;
    XMLVM_EXIT_METHOD()
    return _r0.f;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE java_lang_reflect_AccessibleObject_invokeD___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_AccessibleObject_invokeD___java_lang_Object_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.AccessibleObject", "invokeD", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("AccessibleObject.java", 279)
    _r0.d = 0.0;
    XMLVM_EXIT_METHOD()
    return _r0.d;
    //XMLVM_END_WRAPPER
}

