#include "xmlvm.h"
#include "java_io_File.h"
#include "java_io_IOException.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_RuntimeException.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Throwable.h"
#include "java_lang_reflect_Constructor.h"
#include "java_lang_reflect_Field.h"
#include "java_lang_reflect_Method.h"
#include "org_xmlvm_runtime_XMLVMClassLoader.h"

#include "java_lang_Class.h"

#define XMLVM_CURRENT_CLASS_NAME Class
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Class

__TIB_DEFINITION_java_lang_Class __TIB_java_lang_Class = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Class, // classInitializer
    "java.lang.Class", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/Object;Ljava/io/Serializable;Ljava/lang/reflect/AnnotatedElement;Ljava/lang/reflect/GenericDeclaration;Ljava/lang/reflect/Type;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Class), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Class;
JAVA_OBJECT __CLASS_java_lang_Class_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Class_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Class_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Class_serialVersionUID;
static JAVA_OBJECT _STATIC_java_lang_Class_classLoader;

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

void __INIT_java_lang_Class()
{
    staticInitializerLock(&__TIB_java_lang_Class);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Class.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Class.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Class);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Class.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Class.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Class.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Class")
        __INIT_IMPL_java_lang_Class();
    }
}

void __INIT_IMPL_java_lang_Class()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Class.newInstanceFunc = __NEW_INSTANCE_java_lang_Class;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Class.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Class.vtable[6] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_Class.vtable[7] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_Class.vtable[8] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_Class.vtable[9] = (VTABLE_PTR) &java_lang_Class_getMethod___java_lang_String_java_lang_Class_1ARRAY;
    __TIB_java_lang_Class.vtable[10] = (VTABLE_PTR) &java_lang_Class_getTypeParameters__;
    __TIB_java_lang_Class.vtable[11] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_Class.vtable[12] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_Class.vtable[5] = (VTABLE_PTR) &java_lang_Class_toString__;
    xmlvm_init_native_java_lang_Class();
    // Initialize interface information
    __TIB_java_lang_Class.numImplementedInterfaces = 4;
    __TIB_java_lang_Class.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 4);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Class.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_reflect_AnnotatedElement)

    __TIB_java_lang_Class.implementedInterfaces[0][1] = &__TIB_java_lang_reflect_AnnotatedElement;

    XMLVM_CLASS_INIT(java_lang_reflect_GenericDeclaration)

    __TIB_java_lang_Class.implementedInterfaces[0][2] = &__TIB_java_lang_reflect_GenericDeclaration;

    XMLVM_CLASS_INIT(java_lang_reflect_Type)

    __TIB_java_lang_Class.implementedInterfaces[0][3] = &__TIB_java_lang_reflect_Type;
    // Initialize itable for this class
    __TIB_java_lang_Class.itableBegin = &__TIB_java_lang_Class.itable[0];
    __TIB_java_lang_Class.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotation___java_lang_Class] = __TIB_java_lang_Class.vtable[6];
    __TIB_java_lang_Class.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotations__] = __TIB_java_lang_Class.vtable[7];
    __TIB_java_lang_Class.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getDeclaredAnnotations__] = __TIB_java_lang_Class.vtable[8];
    __TIB_java_lang_Class.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_isAnnotationPresent___java_lang_Class] = __TIB_java_lang_Class.vtable[11];
    __TIB_java_lang_Class.itable[XMLVM_ITABLE_IDX_java_lang_reflect_GenericDeclaration_getTypeParameters__] = __TIB_java_lang_Class.vtable[10];

    _STATIC_java_lang_Class_serialVersionUID = 3206093459760846163;
    _STATIC_java_lang_Class_classLoader = (java_lang_ClassLoader*) JAVA_NULL;

    __TIB_java_lang_Class.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Class.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Class.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Class.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Class.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Class.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Class.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Class.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Class = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Class);
    __TIB_java_lang_Class.clazz = __CLASS_java_lang_Class;
    __TIB_java_lang_Class.baseType = JAVA_NULL;
    __CLASS_java_lang_Class_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Class);
    __CLASS_java_lang_Class_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Class_1ARRAY);
    __CLASS_java_lang_Class_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Class_2ARRAY);
    java_lang_Class___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Class]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Class.classInitialized = 1;
}

void __DELETE_java_lang_Class(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Class]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Class(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Class*) me)->fields.java_lang_Class.tib_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Class]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Class()
{    XMLVM_CLASS_INIT(java_lang_Class)
java_lang_Class* me = (java_lang_Class*) XMLVM_MALLOC(sizeof(java_lang_Class));
    me->tib = &__TIB_java_lang_Class;
    __INIT_INSTANCE_MEMBERS_java_lang_Class(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Class]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Class()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Class_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Class)
    return _STATIC_java_lang_Class_serialVersionUID;
}

void java_lang_Class_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Class)
_STATIC_java_lang_Class_serialVersionUID = v;
}

JAVA_OBJECT java_lang_Class_GET_classLoader()
{
    XMLVM_CLASS_INIT(java_lang_Class)
    return _STATIC_java_lang_Class_classLoader;
}

void java_lang_Class_PUT_classLoader(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Class)
_STATIC_java_lang_Class_classLoader = v;
}

//XMLVM_NATIVE[void java_lang_Class_initNativeLayer__()]

void java_lang_Class___INIT____java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class___INIT____java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Class", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Class.java", 127)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Class.java", 128)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Class*) _r0.o)->fields.java_lang_Class.tib_ = _r1.o;
    XMLVM_SOURCE_POSITION("Class.java", 129)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getStackClasses___int_boolean(JAVA_INT n1, JAVA_BOOLEAN n2)]

JAVA_OBJECT java_lang_Class_forName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_lang_Class)
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_forName___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Class", "forName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Class.java", 189)
    _r0.i = 1;
    _r1.o = JAVA_NULL;
    _r0.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r2.o, _r0.i, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_OBJECT n3)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getClasses__(JAVA_OBJECT me)]

//XMLVM_NATIVE[void java_lang_Class_verify__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getAnnotation___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getAnnotations__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getCanonicalName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getCanonicalName__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getCanonicalName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 277)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_Class_isAnonymousClass__(_r3.o);
    if (_r0.i != 0) goto label12;
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_Class_isLocalClass__(_r3.o);
    if (_r0.i == 0) goto label14;
    label12:;
    XMLVM_SOURCE_POSITION("Class.java", 279)
    _r0.o = JAVA_NULL;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 284)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label14:;
    XMLVM_SOURCE_POSITION("Class.java", 281)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_Class_isPrimitive__(_r3.o);
    if (_r0.i == 0) goto label25;
    XMLVM_SOURCE_POSITION("Class.java", 282)
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_Class_getSimpleName__(_r3.o);
    goto label13;
    label25:;
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_Class_getPackage__(_r3.o);
    if (_r0.o != JAVA_NULL) goto label42;
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label33:;
    _r1.i = 36;
    _r2.i = 46;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replace___char_char(_r0.o, _r1.i, _r2.i);
    goto label13;
    label42:;
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_lang_Class_getPackage__(_r3.o);

    
    // Red class access removed: java.lang.Package::getName
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "."
    _r1.o = xmlvm_create_java_string_from_pool(52);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(3)
    _r1.o = java_lang_Class_getSimpleName__(_r3.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    goto label33;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getClassLoader__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getClassLoader__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getClassLoader", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 301)
    _r0.o = java_lang_Class_GET_classLoader();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getClassLoaderImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getClassLoaderImpl__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getClassLoaderImpl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 317)
    _r0.o = java_lang_Class_GET_classLoader();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getComponentType__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getConstructor___java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getConstructor___java_lang_Class_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Class", "getConstructor", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Class.java", 347)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_Class_getDeclaredConstructor___java_lang_Class_1ARRAY(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getConstructors__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredAnnotations__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredClasses__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getDeclaredConstructor___java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getDeclaredConstructor___java_lang_Class_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Class", "getDeclaredConstructor", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("Class.java", 409)
    XMLVM_CHECK_NPE(5)
    _r0.o = java_lang_Class_getDeclaredConstructors__(_r5.o);
    _r1.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("Class.java", 410)
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i >= _r2.i) goto label26;
    XMLVM_SOURCE_POSITION("Class.java", 411)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(2)
    _r2.o = java_lang_reflect_Constructor_getParameterTypes__(_r2.o);
    _r2.i = java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY(_r6.o, _r2.o);
    if (_r2.i == 0) goto label23;
    XMLVM_SOURCE_POSITION("Class.java", 412)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label23:;
    _r1.i = _r1.i + 1;
    goto label5;
    label26:;
    XMLVM_SOURCE_POSITION("Class.java", 415)

    
    // Red class access removed: java.lang.NoSuchMethodException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r3.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT___(_r3.o);
    XMLVM_CHECK_NPE(5)
    _r4.o = java_lang_Class_getName__(_r5.o);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    // ".<init>"
    _r4.o = xmlvm_create_java_string_from_pool(53);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);

    
    // Red class access removed: java.lang.NoSuchMethodException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r2.o)
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_lang_Class)
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Class", "arrayEqual", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = n1;
    _r6.o = n2;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Class.java", 419)
    if (_r5.o != JAVA_NULL) goto label13;
    XMLVM_SOURCE_POSITION("Class.java", 420)
    if (_r6.o == JAVA_NULL) goto label9;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r1.i != 0) goto label11;
    label9:;
    _r1 = _r4;
    label10:;
    XMLVM_SOURCE_POSITION("Class.java", 437)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label11:;
    _r1 = _r3;
    goto label10;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 423)
    if (_r6.o != JAVA_NULL) goto label22;
    XMLVM_SOURCE_POSITION("Class.java", 424)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r1.i != 0) goto label20;
    _r1 = _r4;
    goto label10;
    label20:;
    _r1 = _r3;
    goto label10;
    label22:;
    XMLVM_SOURCE_POSITION("Class.java", 427)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r1.i == _r2.i) goto label28;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("Class.java", 428)
    goto label10;
    label28:;
    XMLVM_SOURCE_POSITION("Class.java", 431)
    _r0.i = 0;
    label29:;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r0.i >= _r1.i) goto label43;
    XMLVM_SOURCE_POSITION("Class.java", 432)
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r1.o == _r2.o) goto label40;
    _r1 = _r3;
    XMLVM_SOURCE_POSITION("Class.java", 433)
    goto label10;
    label40:;
    _r0.i = _r0.i + 1;
    goto label29;
    label43:;
    _r1 = _r4;
    goto label10;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredConstructors__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredField___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredFields__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getDeclaredMethod___java_lang_String_java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getDeclaredMethod___java_lang_String_java_lang_Class_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Class", "getDeclaredMethod", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = me;
    _r7.o = n1;
    _r8.o = n2;
    XMLVM_SOURCE_POSITION("Class.java", 509)
    XMLVM_CHECK_NPE(6)
    _r2.o = java_lang_Class_getDeclaredMethods__(_r6.o);
    _r0.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("Class.java", 510)
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r0.i >= _r3.i) goto label34;
    XMLVM_SOURCE_POSITION("Class.java", 511)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Class.java", 512)
    //java_lang_reflect_Method_getName__[13]
    XMLVM_CHECK_NPE(1)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_reflect_Method*) _r1.o)->tib->vtable[13])(_r1.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r3.o);
    if (_r3.i == 0) goto label31;
    XMLVM_CHECK_NPE(1)
    _r3.o = java_lang_reflect_Method_getParameterTypes__(_r1.o);
    _r3.i = java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY(_r8.o, _r3.o);
    if (_r3.i == 0) goto label31;
    XMLVM_SOURCE_POSITION("Class.java", 514)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label31:;
    _r0.i = _r0.i + 1;
    goto label5;
    label34:;
    XMLVM_SOURCE_POSITION("Class.java", 517)

    
    // Red class access removed: java.lang.NoSuchMethodException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r4.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT___(_r4.o);
    XMLVM_CHECK_NPE(6)
    _r5.o = java_lang_Class_getName__(_r6.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r5.o);
    // "."
    _r5.o = xmlvm_create_java_string_from_pool(52);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r5.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r7.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r4.o)->tib->vtable[5])(_r4.o);

    
    // Red class access removed: java.lang.NoSuchMethodException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getDeclaredMethods__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getDeclaringClass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getDeclaringClass__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getDeclaringClass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 543)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Class_isArray__(_r1.o);
    if (_r0.i != 0) goto label12;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Class_isPrimitive__(_r1.o);
    if (_r0.i == 0) goto label14;
    label12:;
    XMLVM_SOURCE_POSITION("Class.java", 544)
    _r0.o = JAVA_NULL;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 546)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label14:;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_Class_getEnclosingClass__(_r1.o);
    goto label13;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getEnclosingClass__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getEnclosingConstructor__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getEnclosingMethod__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getEnumConstants__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getField___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getFields__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getGenericInterfaces__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getGenericInterfaces__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getGenericInterfaces", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 632)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 633)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_Class_getSignatureAttribute__(_r2.o);

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::parseForClass
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 634)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser,org.apache.harmony.luni.lang.reflect.ListOfTypes::interfaceTypes
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.Types::getClonedTypeArray
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getGenericSuperclass__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getGenericSuperclass__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getGenericSuperclass", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 646)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 647)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_Class_getSignatureAttribute__(_r2.o);

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::parseForClass
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 648)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser,java.lang.reflect.Type::superclassType
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.Types::getType
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r1.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getInterfaces__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getMethod___java_lang_String_java_lang_Class_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getMethod___java_lang_String_java_lang_Class_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.Class", "getMethod", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r6.o = me;
    _r7.o = n1;
    _r8.o = n2;
    XMLVM_SOURCE_POSITION("Class.java", 684)
    XMLVM_CHECK_NPE(6)
    _r2.o = java_lang_Class_getMethods__(_r6.o);
    _r0.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("Class.java", 685)
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r0.i >= _r3.i) goto label34;
    XMLVM_SOURCE_POSITION("Class.java", 686)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Class.java", 687)
    //java_lang_reflect_Method_getName__[13]
    XMLVM_CHECK_NPE(1)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_reflect_Method*) _r1.o)->tib->vtable[13])(_r1.o);
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(7)
    _r3.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[1])(_r7.o, _r3.o);
    if (_r3.i == 0) goto label31;
    XMLVM_CHECK_NPE(1)
    _r3.o = java_lang_reflect_Method_getParameterTypes__(_r1.o);
    _r3.i = java_lang_Class_arrayEqual___java_lang_Object_1ARRAY_java_lang_Object_1ARRAY(_r8.o, _r3.o);
    if (_r3.i == 0) goto label31;
    XMLVM_SOURCE_POSITION("Class.java", 689)
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label31:;
    _r0.i = _r0.i + 1;
    goto label5;
    label34:;
    XMLVM_SOURCE_POSITION("Class.java", 692)

    
    // Red class access removed: java.lang.NoSuchMethodException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r4.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT___(_r4.o);
    XMLVM_CHECK_NPE(6)
    _r5.o = java_lang_Class_getName__(_r6.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r5.o);
    // "."
    _r5.o = xmlvm_create_java_string_from_pool(52);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r5.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r7.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r4.o)->tib->vtable[5])(_r4.o);

    
    // Red class access removed: java.lang.NoSuchMethodException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getMethods__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getMethods__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getMethods", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.o = me;
    _r6.i = 0;
    XMLVM_SOURCE_POSITION("Class.java", 713)
    XMLVM_CHECK_NPE(7)
    _r3.o = java_lang_Class_getDeclaredMethods__(_r7.o);
    XMLVM_SOURCE_POSITION("Class.java", 714)
    XMLVM_CHECK_NPE(7)
    _r1.o = java_lang_Class_getSuperclass__(_r7.o);
    if (_r1.o != JAVA_NULL) goto label13;
    XMLVM_SOURCE_POSITION("Class.java", 715)
    _r4 = _r3;
    label12:;
    XMLVM_SOURCE_POSITION("Class.java", 716)
    XMLVM_SOURCE_POSITION("Class.java", 722)
    XMLVM_EXIT_METHOD()
    return _r4.o;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 718)
    XMLVM_CHECK_NPE(1)
    _r2.o = java_lang_Class_getMethods__(_r1.o);
    XMLVM_SOURCE_POSITION("Class.java", 719)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r5.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    _r4.i = _r4.i + _r5.i;
    XMLVM_CLASS_INIT(java_lang_reflect_Method)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_reflect_Method, _r4.i);
    XMLVM_SOURCE_POSITION("Class.java", 720)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r6.i, _r0.o, _r6.i, _r4.i);
    XMLVM_SOURCE_POSITION("Class.java", 721)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    _r5.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r6.i, _r0.o, _r4.i, _r5.i);
    _r4 = _r0;
    goto label12;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_Class_getModifiers__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getName__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getSimpleName__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getProtectionDomain__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getPDImpl__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getResource___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getResource___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Class", "getResource", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    volatile XMLVMElem _r9;
    _r8.o = me;
    _r9.o = n1;
    _r6.i = 46;
    // "/"
    _r7.o = xmlvm_create_java_string_from_pool(54);
    XMLVM_SOURCE_POSITION("Class.java", 796)
    _r2.o = __NEW_java_io_File();
    XMLVM_CHECK_NPE(2)
    java_io_File___INIT____java_lang_String(_r2.o, _r9.o);
    XMLVM_SOURCE_POSITION("Class.java", 798)
    XMLVM_CHECK_NPE(2)
    _r4.i = java_io_File_exists__(_r2.o);
    if (_r4.i != 0) goto label33;
    XMLVM_SOURCE_POSITION("Class.java", 800)
    // "/"
    _r4.o = xmlvm_create_java_string_from_pool(54);
    XMLVM_CHECK_NPE(9)
    _r4.i = java_lang_String_startsWith___java_lang_String(_r9.o, _r7.o);
    if (_r4.i == 0) goto label68;
    XMLVM_SOURCE_POSITION("Class.java", 801)
    _r4.i = 1;
    XMLVM_CHECK_NPE(9)
    _r9.o = java_lang_String_substring___int(_r9.o, _r4.i);
    label28:;
    XMLVM_SOURCE_POSITION("Class.java", 813)
    _r2.o = __NEW_java_io_File();
    XMLVM_CHECK_NPE(2)
    java_io_File___INIT____java_lang_String(_r2.o, _r9.o);
    label33:;
    XMLVM_SOURCE_POSITION("Class.java", 816)
    XMLVM_CHECK_NPE(2)
    _r4.i = java_io_File_exists__(_r2.o);
    if (_r4.i == 0) goto label125;
    XMLVM_TRY_BEGIN(w556aaac44b1c26)
    // Begin try
    XMLVM_SOURCE_POSITION("Class.java", 818)

    
    // Red class access removed: java.net.URL::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r5.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(5)
    java_lang_StringBuilder___INIT___(_r5.o);
    // "file://"
    _r6.o = xmlvm_create_java_string_from_pool(55);
    XMLVM_CHECK_NPE(5)
    _r5.o = java_lang_StringBuilder_append___java_lang_String(_r5.o, _r6.o);
    XMLVM_CHECK_NPE(2)
    _r6.o = java_io_File_getAbsolutePath__(_r2.o);
    XMLVM_CHECK_NPE(5)
    _r5.o = java_lang_StringBuilder_append___java_lang_String(_r5.o, _r6.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(5)
    _r5.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r5.o)->tib->vtable[5])(_r5.o);

    
    // Red class access removed: java.net.URL::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w556aaac44b1c26)
    XMLVM_CATCH_END(w556aaac44b1c26)
    XMLVM_RESTORE_EXCEPTION_ENV(w556aaac44b1c26)
    label67:;
    XMLVM_SOURCE_POSITION("Class.java", 823)
    XMLVM_EXIT_METHOD()
    return _r4.o;
    label68:;
    XMLVM_SOURCE_POSITION("Class.java", 803)
    XMLVM_CHECK_NPE(8)
    _r3.o = java_lang_Class_getName__(_r8.o);
    XMLVM_SOURCE_POSITION("Class.java", 804)
    XMLVM_CHECK_NPE(3)
    _r0.i = java_lang_String_lastIndexOf___int(_r3.o, _r6.i);
    _r4.i = -1;
    if (_r0.i == _r4.i) goto label114;
    XMLVM_SOURCE_POSITION("Class.java", 805)
    XMLVM_SOURCE_POSITION("Class.java", 806)
    _r4.i = 0;
    XMLVM_CHECK_NPE(3)
    _r4.o = java_lang_String_substring___int_int(_r3.o, _r4.i, _r0.i);
    _r5.i = 47;
    XMLVM_CHECK_NPE(4)
    _r3.o = java_lang_String_replace___char_char(_r4.o, _r6.i, _r5.i);
    label90:;
    XMLVM_SOURCE_POSITION("Class.java", 811)
    _r4.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT___(_r4.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r3.o);
    // "/"
    _r5.o = xmlvm_create_java_string_from_pool(54);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r7.o);
    XMLVM_CHECK_NPE(4)
    _r4.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r9.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(4)
    _r9.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r4.o)->tib->vtable[5])(_r4.o);
    goto label28;
    label114:;
    XMLVM_SOURCE_POSITION("Class.java", 808)
    // ""
    _r3.o = xmlvm_create_java_string_from_pool(21);
    goto label90;
    label117:;
    XMLVM_SOURCE_POSITION("Class.java", 819)
    java_lang_Thread* curThread_w556aaac44b1c59 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r4.o = curThread_w556aaac44b1c59->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r4;
    XMLVM_SOURCE_POSITION("Class.java", 820)
    _r4.o = __NEW_java_lang_RuntimeException();
    XMLVM_CHECK_NPE(4)
    java_lang_RuntimeException___INIT____java_lang_Throwable(_r4.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r4.o)
    label125:;
    _r4.o = JAVA_NULL;
    goto label67;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getResourceAsStream___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getResourceAsStream___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.Class", "getResourceAsStream", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Class.java", 842)
    XMLVM_CHECK_NPE(3)
    _r1.o = java_lang_Class_getResource___java_lang_String(_r3.o, _r4.o);
    if (_r1.o != JAVA_NULL) goto label8;
    label7:;
    XMLVM_SOURCE_POSITION("Class.java", 843)
    XMLVM_SOURCE_POSITION("Class.java", 849)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label8:;
    XMLVM_TRY_BEGIN(w556aaac45b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Class.java", 847)

    
    // Red class access removed: java.net.URL::openStream
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w556aaac45b1c12)
        XMLVM_CATCH_SPECIFIC(w556aaac45b1c12,java_io_IOException,13)
    XMLVM_CATCH_END(w556aaac45b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w556aaac45b1c12)
    goto label7;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 848)
    java_lang_Thread* curThread_w556aaac45b1c16 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w556aaac45b1c16->fields.java_lang_Thread.xmlvmException_;
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getSigners__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getSuperclass__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_getTypeParameters__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getTypeParameters__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getTypeParameters", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 885)
    java_lang_Object_acquireLockRecursive__(_r2.o);
    XMLVM_TRY_BEGIN(w556aaac48b1b4)
    // Begin try

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 886)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_Class_getSignatureAttribute__(_r2.o);

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::parseForClass
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Class.java", 887)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser,java.lang.reflect.TypeVariable[]::formalTypeParameters
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.reflect.TypeVariable[]::clone
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.reflect.TypeVariable[]::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w556aaac48b1b4)
        XMLVM_CATCH_SPECIFIC(w556aaac48b1b4,java_lang_Object,24)
    XMLVM_CATCH_END(w556aaac48b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w556aaac48b1b4)
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label24:;
    java_lang_Thread* curThread_w556aaac48b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w556aaac48b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r2.o);
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isAnnotation__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isAnnotationPresent___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isAnonymousClass__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isArray__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isAssignableFrom___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isEnum__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isInstance___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isInterface__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isLocalClass__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isMemberClass__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isPrimitive__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_isSynthetic__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_newInstance__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_Class_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_toString__]
    XMLVM_ENTER_METHOD("java.lang.Class", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 1031)
    _r0.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Class.java", 1032)
    XMLVM_CHECK_NPE(2)
    _r1.i = java_lang_Class_isPrimitive__(_r2.o);
    if (_r1.i != 0) goto label27;
    XMLVM_SOURCE_POSITION("Class.java", 1033)
    XMLVM_CHECK_NPE(2)
    _r1.i = java_lang_Class_isInterface__(_r2.o);
    if (_r1.i == 0) goto label39;
    XMLVM_SOURCE_POSITION("Class.java", 1034)
    // "interface"
    _r1.o = xmlvm_create_java_string_from_pool(56);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label22:;
    XMLVM_SOURCE_POSITION("Class.java", 1038)
    // " "
    _r1.o = xmlvm_create_java_string_from_pool(57);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    label27:;
    XMLVM_SOURCE_POSITION("Class.java", 1040)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_Class_getName__(_r2.o);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Class.java", 1041)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r1.o;
    label39:;
    XMLVM_SOURCE_POSITION("Class.java", 1036)
    // "class"
    _r1.o = xmlvm_create_java_string_from_pool(58);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    goto label22;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Class_getPackage__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_getPackage__]
    XMLVM_ENTER_METHOD("java.lang.Class", "getPackage", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Class.java", 1053)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_Class_getPackageString__(_r1.o);

    
    // Red class access removed: java.lang.Package::getPackage
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getPackageString__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_Class_desiredAssertionStatus__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_asSubclass___java_lang_Class(JAVA_OBJECT me, JAVA_OBJECT n1)]

JAVA_OBJECT java_lang_Class_cast___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class_cast___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Class", "cast", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Class.java", 1092)
    if (_r2.o == JAVA_NULL) goto label12;
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r2.o)->tib->vtable[3])(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Class_isAssignableFrom___java_lang_Class(_r1.o, _r0.o);
    if (_r0.i == 0) goto label13;
    label12:;
    XMLVM_SOURCE_POSITION("Class.java", 1093)
    XMLVM_EXIT_METHOD()
    return _r2.o;
    label13:;
    XMLVM_SOURCE_POSITION("Class.java", 1095)

    
    // Red class access removed: java.lang.ClassCastException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.ClassCastException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_Class_getSignatureAttribute__(JAVA_OBJECT me)]

void java_lang_Class___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Class___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Class", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Class.java", 118)
    _r0.o = __NEW_org_xmlvm_runtime_XMLVMClassLoader();
    XMLVM_CHECK_NPE(0)
    org_xmlvm_runtime_XMLVMClassLoader___INIT___(_r0.o);
    java_lang_Class_PUT_classLoader( _r0.o);
    XMLVM_SOURCE_POSITION("Class.java", 124)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

