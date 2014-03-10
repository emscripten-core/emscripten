#include "xmlvm.h"
#include "java_lang_Class.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_reflect_Type.h"

#include "java_lang_reflect_Method.h"

#define XMLVM_CURRENT_CLASS_NAME Method
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_reflect_Method

__TIB_DEFINITION_java_lang_reflect_Method __TIB_java_lang_reflect_Method = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_reflect_Method, // classInitializer
    "java.lang.reflect.Method", // className
    "java.lang.reflect", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "<T:Ljava/lang/Object;>Ljava/lang/reflect/AccessibleObject;Ljava/lang/reflect/GenericDeclaration;Ljava/lang/reflect/Member;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_reflect_AccessibleObject, // extends
    sizeof(java_lang_reflect_Method), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_reflect_Method;
JAVA_OBJECT __CLASS_java_lang_reflect_Method_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Method_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_reflect_Method_3ARRAY;
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

void __INIT_java_lang_reflect_Method()
{
    staticInitializerLock(&__TIB_java_lang_reflect_Method);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_reflect_Method.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_reflect_Method.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_reflect_Method);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_reflect_Method.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_reflect_Method.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_reflect_Method.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.reflect.Method")
        __INIT_IMPL_java_lang_reflect_Method();
    }
}

void __INIT_IMPL_java_lang_reflect_Method()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_reflect_AccessibleObject)
    __TIB_java_lang_reflect_Method.newInstanceFunc = __NEW_INSTANCE_java_lang_reflect_Method;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_reflect_Method.vtable, __TIB_java_lang_reflect_AccessibleObject.vtable, sizeof(__TIB_java_lang_reflect_AccessibleObject.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_reflect_Method.vtable[14] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[16] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[1] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[12] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[9] = (VTABLE_PTR) &java_lang_reflect_Method_getModifiers__;
    __TIB_java_lang_reflect_Method.vtable[13] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[4] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[15] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_java_lang_reflect_Method.vtable[5] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    xmlvm_init_native_java_lang_reflect_Method();
    // Initialize interface information
    __TIB_java_lang_reflect_Method.numImplementedInterfaces = 3;
    __TIB_java_lang_reflect_Method.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_reflect_AnnotatedElement)

    __TIB_java_lang_reflect_Method.implementedInterfaces[0][0] = &__TIB_java_lang_reflect_AnnotatedElement;

    XMLVM_CLASS_INIT(java_lang_reflect_GenericDeclaration)

    __TIB_java_lang_reflect_Method.implementedInterfaces[0][1] = &__TIB_java_lang_reflect_GenericDeclaration;

    XMLVM_CLASS_INIT(java_lang_reflect_Member)

    __TIB_java_lang_reflect_Method.implementedInterfaces[0][2] = &__TIB_java_lang_reflect_Member;
    // Initialize itable for this class
    __TIB_java_lang_reflect_Method.itableBegin = &__TIB_java_lang_reflect_Method.itable[0];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotation___java_lang_Class] = __TIB_java_lang_reflect_Method.vtable[6];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getAnnotations__] = __TIB_java_lang_reflect_Method.vtable[7];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_getDeclaredAnnotations__] = __TIB_java_lang_reflect_Method.vtable[8];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_AnnotatedElement_isAnnotationPresent___java_lang_Class] = __TIB_java_lang_reflect_Method.vtable[11];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_GenericDeclaration_getTypeParameters__] = __TIB_java_lang_reflect_Method.vtable[14];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_Member_getDeclaringClass__] = __TIB_java_lang_reflect_Method.vtable[12];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_Member_getModifiers__] = __TIB_java_lang_reflect_Method.vtable[9];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_Member_getName__] = __TIB_java_lang_reflect_Method.vtable[13];
    __TIB_java_lang_reflect_Method.itable[XMLVM_ITABLE_IDX_java_lang_reflect_Member_isSynthetic__] = __TIB_java_lang_reflect_Method.vtable[16];


    __TIB_java_lang_reflect_Method.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_reflect_Method.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_reflect_Method.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_reflect_Method.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_reflect_Method.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_reflect_Method.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_reflect_Method.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_reflect_Method.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_reflect_Method = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_reflect_Method);
    __TIB_java_lang_reflect_Method.clazz = __CLASS_java_lang_reflect_Method;
    __TIB_java_lang_reflect_Method.baseType = JAVA_NULL;
    __CLASS_java_lang_reflect_Method_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Method);
    __CLASS_java_lang_reflect_Method_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Method_1ARRAY);
    __CLASS_java_lang_reflect_Method_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_reflect_Method_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_reflect_Method]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_reflect_Method.classInitialized = 1;
}

void __DELETE_java_lang_reflect_Method(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_reflect_Method]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_reflect_Method(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_reflect_AccessibleObject(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.name_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.clazz_ = (java_lang_Class*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.address_ = (java_lang_Object*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.slot_ = 0;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.parameterTypes_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.exceptionTypes_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.modifiers_ = 0;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.signature_ = (java_lang_String*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.annotations_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_reflect_Method*) me)->fields.java_lang_reflect_Method.parameterAnnotations_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_reflect_Method]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_reflect_Method()
{    XMLVM_CLASS_INIT(java_lang_reflect_Method)
java_lang_reflect_Method* me = (java_lang_reflect_Method*) XMLVM_MALLOC(sizeof(java_lang_reflect_Method));
    me->tib = &__TIB_java_lang_reflect_Method;
    __INIT_INSTANCE_MEMBERS_java_lang_reflect_Method(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_reflect_Method]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_reflect_Method()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_lang_reflect_Method___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Method___INIT___]
    XMLVM_ENTER_METHOD("java.lang.reflect.Method", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("Method.java", 49)
    XMLVM_CHECK_NPE(0)
    java_lang_reflect_AccessibleObject___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Method.java", 51)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_reflect_Method___INIT____java_lang_String_java_lang_Class_java_lang_Class_1ARRAY_java_lang_Class_1ARRAY_int_java_lang_Object_int_java_lang_String_byte_1ARRAY_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_INT n5, JAVA_OBJECT n6, JAVA_INT n7, JAVA_OBJECT n8, JAVA_OBJECT n9, JAVA_OBJECT n10)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Method___INIT____java_lang_String_java_lang_Class_java_lang_Class_1ARRAY_java_lang_Class_1ARRAY_int_java_lang_Object_int_java_lang_String_byte_1ARRAY_byte_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.reflect.Method", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    _r0.o = me;
    _r1.o = n1;
    _r2.o = n2;
    _r3.o = n3;
    _r4.o = n4;
    _r5.i = n5;
    _r6.o = n6;
    _r7.i = n7;
    _r8.o = n8;
    _r9.o = n9;
    _r10.o = n10;
    XMLVM_SOURCE_POSITION("Method.java", 55)
    XMLVM_CHECK_NPE(0)
    java_lang_reflect_AccessibleObject___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Method.java", 56)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.name_ = _r1.o;
    XMLVM_SOURCE_POSITION("Method.java", 57)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.clazz_ = _r2.o;
    XMLVM_SOURCE_POSITION("Method.java", 58)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.parameterTypes_ = _r3.o;
    XMLVM_SOURCE_POSITION("Method.java", 59)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.exceptionTypes_ = _r4.o;
    XMLVM_SOURCE_POSITION("Method.java", 60)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.modifiers_ = _r5.i;
    XMLVM_SOURCE_POSITION("Method.java", 61)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.address_ = _r6.o;
    XMLVM_SOURCE_POSITION("Method.java", 62)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.slot_ = _r7.i;
    XMLVM_SOURCE_POSITION("Method.java", 63)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.signature_ = _r8.o;
    XMLVM_SOURCE_POSITION("Method.java", 64)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.annotations_ = _r9.o;
    XMLVM_SOURCE_POSITION("Method.java", 65)
    XMLVM_CHECK_NPE(0)
    ((java_lang_reflect_Method*) _r0.o)->fields.java_lang_reflect_Method.parameterAnnotations_ = _r10.o;
    XMLVM_SOURCE_POSITION("Method.java", 66)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getTypeParameters__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_toGenericString__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getGenericParameterTypes__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getGenericExceptionTypes__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getGenericReturnType__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getParameterAnnotations__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_Method_isVarArgs__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_Method_isBridge__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_Method_isSynthetic__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getDefaultValue__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_BOOLEAN java_lang_reflect_Method_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getDeclaringClass__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getExceptionTypes__(JAVA_OBJECT me)]

JAVA_INT java_lang_reflect_Method_getModifiers__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Method_getModifiers__]
    XMLVM_ENTER_METHOD("java.lang.reflect.Method", "getModifiers", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Method.java", 216)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_reflect_Method*) _r1.o)->fields.java_lang_reflect_Method.modifiers_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_getName__(JAVA_OBJECT me)]

JAVA_OBJECT java_lang_reflect_Method_getParameterTypes__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Method_getParameterTypes__]
    XMLVM_ENTER_METHOD("java.lang.reflect.Method", "getParameterTypes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Method.java", 235)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_reflect_Method*) _r1.o)->fields.java_lang_reflect_Method.parameterTypes_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_reflect_Method_getReturnType__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_reflect_Method_getReturnType__]
    XMLVM_ENTER_METHOD("java.lang.reflect.Method", "getReturnType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.o = me;
    XMLVM_SOURCE_POSITION("Method.java", 245)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = JAVA_NULL;

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Method.java", 246)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_reflect_Method*) _r3.o)->fields.java_lang_reflect_Method.signature_;
    XMLVM_CHECK_NPE(3)
    _r2.o = ((java_lang_reflect_Method*) _r3.o)->fields.java_lang_reflect_Method.exceptionTypes_;

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser::parseForMethod
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Method.java", 247)

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.GenericSignatureParser,java.lang.reflect.Type::returnType
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: org.apache.harmony.luni.lang.reflect.Types::getType
    XMLVM_RED_CLASS_DEPENDENCY();
    _r3.o = _r3.o;
    XMLVM_EXIT_METHOD()
    return _r3.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT java_lang_reflect_Method_hashCode__(JAVA_OBJECT me)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_invoke___java_lang_Object_java_lang_Object_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)]

//XMLVM_NATIVE[JAVA_OBJECT java_lang_reflect_Method_toString__(JAVA_OBJECT me)]

