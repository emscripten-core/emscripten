#include "xmlvm.h"
#include "java_lang_Class.h"

#include "org_xmlvm_runtime_XMLVMArray.h"

#define XMLVM_CURRENT_CLASS_NAME XMLVMArray
#define XMLVM_CURRENT_PKG_CLASS_NAME org_xmlvm_runtime_XMLVMArray

__TIB_DEFINITION_org_xmlvm_runtime_XMLVMArray __TIB_org_xmlvm_runtime_XMLVMArray = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_xmlvm_runtime_XMLVMArray, // classInitializer
    "org.xmlvm.runtime.XMLVMArray", // className
    "org.xmlvm.runtime", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_xmlvm_runtime_XMLVMArray), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_1ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_2ARRAY;
JAVA_OBJECT __CLASS_org_xmlvm_runtime_XMLVMArray_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_org_xmlvm_runtime_XMLVMArray_serialVersionUID;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
    {"serialVersionUID",
    &__CLASS_long,
    0 | java_lang_reflect_Modifier_PRIVATE | java_lang_reflect_Modifier_STATIC,
    0,
    &_STATIC_org_xmlvm_runtime_XMLVMArray_serialVersionUID,
    "",
    JAVA_NULL},
    {"type",
    &__CLASS_java_lang_Class,
    0 | java_lang_reflect_Modifier_PRIVATE,
    XMLVM_OFFSETOF(org_xmlvm_runtime_XMLVMArray, fields.org_xmlvm_runtime_XMLVMArray.type_),
    0,
    "",
    JAVA_NULL},
    {"length",
    &__CLASS_int,
    0 | java_lang_reflect_Modifier_PRIVATE,
    XMLVM_OFFSETOF(org_xmlvm_runtime_XMLVMArray, fields.org_xmlvm_runtime_XMLVMArray.length_),
    0,
    "",
    JAVA_NULL},
    {"array",
    &__CLASS_java_lang_Object,
    0 | java_lang_reflect_Modifier_PRIVATE,
    XMLVM_OFFSETOF(org_xmlvm_runtime_XMLVMArray, fields.org_xmlvm_runtime_XMLVMArray.array_),
    0,
    "",
    JAVA_NULL},
};

static JAVA_OBJECT* __constructor0_arg_types[] = {
    &__CLASS_java_lang_Class,
    &__CLASS_int,
    &__CLASS_java_lang_Object,
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
    {&__constructor0_arg_types[0],
    sizeof(__constructor0_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/lang/Class;ILjava/lang/Object;)V",
    JAVA_NULL,
    JAVA_NULL},
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    JAVA_OBJECT obj = __NEW_org_xmlvm_runtime_XMLVMArray();
    java_lang_reflect_Constructor* c = (java_lang_reflect_Constructor*) constructor;
    org_xmlvm_runtime_XMLVMArray* args = (org_xmlvm_runtime_XMLVMArray*) arguments;
    JAVA_ARRAY_OBJECT* argsArray = (JAVA_ARRAY_OBJECT*) args->fields.org_xmlvm_runtime_XMLVMArray.array_;
    switch (c->fields.java_lang_reflect_Constructor.slot_) {
    case 0:
        org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object(obj, argsArray[0], ((java_lang_Integer*) argsArray[1])->fields.java_lang_Integer.value_, argsArray[2]);
        break;
    default:
        XMLVM_INTERNAL_ERROR();
        break;
    }
    return obj;
}

static JAVA_OBJECT* __method0_arg_types[] = {
};

static JAVA_OBJECT* __method1_arg_types[] = {
    &__CLASS_java_lang_Class,
    &__CLASS_int,
};

static JAVA_OBJECT* __method2_arg_types[] = {
    &__CLASS_java_lang_Class,
    &__CLASS_int,
    &__CLASS_java_lang_Object,
};

static JAVA_OBJECT* __method3_arg_types[] = {
    &__CLASS_java_lang_Class,
    &__CLASS_org_xmlvm_runtime_XMLVMArray,
};

static JAVA_OBJECT* __method4_arg_types[] = {
    &__CLASS_org_xmlvm_runtime_XMLVMArray,
    &__CLASS_java_lang_Object,
};

static JAVA_OBJECT* __method5_arg_types[] = {
};

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
    {"initNativeLayer",
    &__method0_arg_types[0],
    sizeof(__method0_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "()V",
    JAVA_NULL,
    JAVA_NULL},
    {"createSingleDimension",
    &__method1_arg_types[0],
    sizeof(__method1_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/lang/Class;I)Lorg/xmlvm/runtime/XMLVMArray;",
    JAVA_NULL,
    JAVA_NULL},
    {"createSingleDimensionWithData",
    &__method2_arg_types[0],
    sizeof(__method2_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/lang/Class;ILjava/lang/Object;)Lorg/xmlvm/runtime/XMLVMArray;",
    JAVA_NULL,
    JAVA_NULL},
    {"createMultiDimensions",
    &__method3_arg_types[0],
    sizeof(__method3_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Ljava/lang/Class;Lorg/xmlvm/runtime/XMLVMArray;)Lorg/xmlvm/runtime/XMLVMArray;",
    JAVA_NULL,
    JAVA_NULL},
    {"fillArray",
    &__method4_arg_types[0],
    sizeof(__method4_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "(Lorg/xmlvm/runtime/XMLVMArray;Ljava/lang/Object;)V",
    JAVA_NULL,
    JAVA_NULL},
    {"clone",
    &__method5_arg_types[0],
    sizeof(__method5_arg_types) / sizeof(JAVA_OBJECT*),
    JAVA_NULL,
    0,
    0,
    "()Ljava/lang/Object;",
    JAVA_NULL,
    JAVA_NULL},
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    JAVA_OBJECT result = JAVA_NULL;
    java_lang_Object* obj = receiver;
    java_lang_reflect_Method* m = (java_lang_reflect_Method*) method;
    org_xmlvm_runtime_XMLVMArray* args = (org_xmlvm_runtime_XMLVMArray*) arguments;
    JAVA_ARRAY_OBJECT* argsArray = (JAVA_ARRAY_OBJECT*) args->fields.org_xmlvm_runtime_XMLVMArray.array_;
    XMLVMElem conversion;
    switch (m->fields.java_lang_reflect_Method.slot_) {
    case 0:
        org_xmlvm_runtime_XMLVMArray_initNativeLayer__();
        break;
    case 1:
        result = (JAVA_OBJECT) org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(argsArray[0], ((java_lang_Integer*) argsArray[1])->fields.java_lang_Integer.value_);
        break;
    case 2:
        result = (JAVA_OBJECT) org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(argsArray[0], ((java_lang_Integer*) argsArray[1])->fields.java_lang_Integer.value_, argsArray[2]);
        break;
    case 3:
        result = (JAVA_OBJECT) org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray(argsArray[0], argsArray[1]);
        break;
    case 4:
        org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object(argsArray[0], argsArray[1]);
        break;
    case 5:
        result = (JAVA_OBJECT) org_xmlvm_runtime_XMLVMArray_clone__(receiver);
        break;
    default:
        XMLVM_INTERNAL_ERROR();
        break;
    }
    return result;
}

void __INIT_org_xmlvm_runtime_XMLVMArray()
{
    staticInitializerLock(&__TIB_org_xmlvm_runtime_XMLVMArray);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_xmlvm_runtime_XMLVMArray.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_xmlvm_runtime_XMLVMArray.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_xmlvm_runtime_XMLVMArray);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_xmlvm_runtime_XMLVMArray.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_xmlvm_runtime_XMLVMArray.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_xmlvm_runtime_XMLVMArray.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.xmlvm.runtime.XMLVMArray")
        __INIT_IMPL_org_xmlvm_runtime_XMLVMArray();
    }
}

void __INIT_IMPL_org_xmlvm_runtime_XMLVMArray()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_xmlvm_runtime_XMLVMArray.newInstanceFunc = __NEW_INSTANCE_org_xmlvm_runtime_XMLVMArray;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_xmlvm_runtime_XMLVMArray.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_org_xmlvm_runtime_XMLVMArray.vtable[0] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    xmlvm_init_native_org_xmlvm_runtime_XMLVMArray();
    // Initialize interface information
    __TIB_org_xmlvm_runtime_XMLVMArray.numImplementedInterfaces = 2;
    __TIB_org_xmlvm_runtime_XMLVMArray.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_org_xmlvm_runtime_XMLVMArray.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_org_xmlvm_runtime_XMLVMArray.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;
    // Initialize itable for this class
    __TIB_org_xmlvm_runtime_XMLVMArray.itableBegin = &__TIB_org_xmlvm_runtime_XMLVMArray.itable[0];

    _STATIC_org_xmlvm_runtime_XMLVMArray_serialVersionUID = -7775139464511217031;

    __TIB_org_xmlvm_runtime_XMLVMArray.declaredFields = &__field_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMArray.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMArray.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMArray.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMArray.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_xmlvm_runtime_XMLVMArray.methodDispatcherFunc = method_dispatcher;
    __TIB_org_xmlvm_runtime_XMLVMArray.declaredMethods = &__method_reflection_data[0];
    __TIB_org_xmlvm_runtime_XMLVMArray.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_xmlvm_runtime_XMLVMArray = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_xmlvm_runtime_XMLVMArray);
    __TIB_org_xmlvm_runtime_XMLVMArray.clazz = __CLASS_org_xmlvm_runtime_XMLVMArray;
    __TIB_org_xmlvm_runtime_XMLVMArray.baseType = JAVA_NULL;
    __CLASS_org_xmlvm_runtime_XMLVMArray_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMArray);
    __CLASS_org_xmlvm_runtime_XMLVMArray_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMArray_1ARRAY);
    __CLASS_org_xmlvm_runtime_XMLVMArray_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_xmlvm_runtime_XMLVMArray_2ARRAY);
    org_xmlvm_runtime_XMLVMArray___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_xmlvm_runtime_XMLVMArray]
    //XMLVM_END_WRAPPER

    __TIB_org_xmlvm_runtime_XMLVMArray.classInitialized = 1;
}

void __DELETE_org_xmlvm_runtime_XMLVMArray(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_xmlvm_runtime_XMLVMArray]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMArray(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_xmlvm_runtime_XMLVMArray*) me)->fields.org_xmlvm_runtime_XMLVMArray.type_ = (java_lang_Class*) JAVA_NULL;
    ((org_xmlvm_runtime_XMLVMArray*) me)->fields.org_xmlvm_runtime_XMLVMArray.length_ = 0;
    ((org_xmlvm_runtime_XMLVMArray*) me)->fields.org_xmlvm_runtime_XMLVMArray.array_ = (java_lang_Object*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMArray]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_xmlvm_runtime_XMLVMArray()
{    XMLVM_CLASS_INIT(org_xmlvm_runtime_XMLVMArray)
org_xmlvm_runtime_XMLVMArray* me = (org_xmlvm_runtime_XMLVMArray*) XMLVM_MALLOC(sizeof(org_xmlvm_runtime_XMLVMArray));
    me->tib = &__TIB_org_xmlvm_runtime_XMLVMArray;
    __INIT_INSTANCE_MEMBERS_org_xmlvm_runtime_XMLVMArray(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_xmlvm_runtime_XMLVMArray]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_xmlvm_runtime_XMLVMArray()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG org_xmlvm_runtime_XMLVMArray_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_XMLVMArray)
    return _STATIC_org_xmlvm_runtime_XMLVMArray_serialVersionUID;
}

void org_xmlvm_runtime_XMLVMArray_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(org_xmlvm_runtime_XMLVMArray)
_STATIC_org_xmlvm_runtime_XMLVMArray_serialVersionUID = v;
}

//XMLVM_NATIVE[void org_xmlvm_runtime_XMLVMArray_initNativeLayer__()]

void org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMArray", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r0.o = me;
    _r1.o = n1;
    _r2.i = n2;
    _r3.o = n3;
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 64)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 65)
    XMLVM_CHECK_NPE(0)
    ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.type_ = _r1.o;
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 66)
    XMLVM_CHECK_NPE(0)
    ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.length_ = _r2.i;
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 67)
    XMLVM_CHECK_NPE(0)
    ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_ = _r3.o;
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 68)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)]

//XMLVM_NATIVE[JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)]

//XMLVM_NATIVE[JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray(JAVA_OBJECT n1, JAVA_OBJECT n2)]

//XMLVM_NATIVE[void org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)]

//XMLVM_NATIVE[JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_clone__(JAVA_OBJECT me)]

void org_xmlvm_runtime_XMLVMArray___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_xmlvm_runtime_XMLVMArray___CLINIT___]
    XMLVM_ENTER_METHOD("org.xmlvm.runtime.XMLVMArray", "<clinit>", "?")
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 60)
    org_xmlvm_runtime_XMLVMArray_initNativeLayer__();
    XMLVM_SOURCE_POSITION("XMLVMArray.java", 61)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

