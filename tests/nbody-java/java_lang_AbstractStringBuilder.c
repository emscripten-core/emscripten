#include "xmlvm.h"
#include "java_io_InvalidObjectException.h"
#include "java_lang_CharSequence.h"
#include "java_lang_Character.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"

#include "java_lang_AbstractStringBuilder.h"

#define XMLVM_CURRENT_CLASS_NAME AbstractStringBuilder
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_AbstractStringBuilder

__TIB_DEFINITION_java_lang_AbstractStringBuilder __TIB_java_lang_AbstractStringBuilder = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_AbstractStringBuilder, // classInitializer
    "java.lang.AbstractStringBuilder", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_AbstractStringBuilder), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder;
JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_AbstractStringBuilder_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_java_lang_AbstractStringBuilder_INITIAL_CAPACITY;

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

void __INIT_java_lang_AbstractStringBuilder()
{
    staticInitializerLock(&__TIB_java_lang_AbstractStringBuilder);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_AbstractStringBuilder.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_AbstractStringBuilder.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_AbstractStringBuilder);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_AbstractStringBuilder.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_AbstractStringBuilder.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_AbstractStringBuilder.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.AbstractStringBuilder")
        __INIT_IMPL_java_lang_AbstractStringBuilder();
    }
}

void __INIT_IMPL_java_lang_AbstractStringBuilder()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_AbstractStringBuilder.newInstanceFunc = __NEW_INSTANCE_java_lang_AbstractStringBuilder;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_AbstractStringBuilder.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_AbstractStringBuilder.vtable[5] = (VTABLE_PTR) &java_lang_AbstractStringBuilder_toString__;
    // Initialize interface information
    __TIB_java_lang_AbstractStringBuilder.numImplementedInterfaces = 0;
    __TIB_java_lang_AbstractStringBuilder.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_java_lang_AbstractStringBuilder_INITIAL_CAPACITY = 16;

    __TIB_java_lang_AbstractStringBuilder.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_AbstractStringBuilder.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_AbstractStringBuilder.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_AbstractStringBuilder.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_AbstractStringBuilder.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_AbstractStringBuilder.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_AbstractStringBuilder.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_AbstractStringBuilder.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_AbstractStringBuilder = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_AbstractStringBuilder);
    __TIB_java_lang_AbstractStringBuilder.clazz = __CLASS_java_lang_AbstractStringBuilder;
    __TIB_java_lang_AbstractStringBuilder.baseType = JAVA_NULL;
    __CLASS_java_lang_AbstractStringBuilder_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_AbstractStringBuilder);
    __CLASS_java_lang_AbstractStringBuilder_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_AbstractStringBuilder_1ARRAY);
    __CLASS_java_lang_AbstractStringBuilder_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_AbstractStringBuilder_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_AbstractStringBuilder]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_AbstractStringBuilder.classInitialized = 1;
}

void __DELETE_java_lang_AbstractStringBuilder(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_AbstractStringBuilder]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_AbstractStringBuilder(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_AbstractStringBuilder*) me)->fields.java_lang_AbstractStringBuilder.value_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((java_lang_AbstractStringBuilder*) me)->fields.java_lang_AbstractStringBuilder.count_ = 0;
    ((java_lang_AbstractStringBuilder*) me)->fields.java_lang_AbstractStringBuilder.shared_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_AbstractStringBuilder]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_AbstractStringBuilder()
{    XMLVM_CLASS_INIT(java_lang_AbstractStringBuilder)
java_lang_AbstractStringBuilder* me = (java_lang_AbstractStringBuilder*) XMLVM_MALLOC(sizeof(java_lang_AbstractStringBuilder));
    me->tib = &__TIB_java_lang_AbstractStringBuilder;
    __INIT_INSTANCE_MEMBERS_java_lang_AbstractStringBuilder(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_AbstractStringBuilder]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_AbstractStringBuilder()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_lang_AbstractStringBuilder();
    java_lang_AbstractStringBuilder___INIT___(me);
    return me;
}

JAVA_INT java_lang_AbstractStringBuilder_GET_INITIAL_CAPACITY()
{
    XMLVM_CLASS_INIT(java_lang_AbstractStringBuilder)
    return _STATIC_java_lang_AbstractStringBuilder_INITIAL_CAPACITY;
}

void java_lang_AbstractStringBuilder_PUT_INITIAL_CAPACITY(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_AbstractStringBuilder)
_STATIC_java_lang_AbstractStringBuilder_INITIAL_CAPACITY = v;
}

JAVA_OBJECT java_lang_AbstractStringBuilder_getValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_getValue__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "getValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 48)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_AbstractStringBuilder_shareValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_shareValue__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "shareValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 55)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 56)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_set___char_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_set___char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "set", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 63)
    if (_r4.o != JAVA_NULL) goto label27;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 64)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    label5:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 66)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i >= _r5.i) goto label20;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 67)
    _r0.o = __NEW_java_io_InvalidObjectException();
    // "luni.4A"
    _r1.o = xmlvm_create_java_string_from_pool(60);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_io_InvalidObjectException___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label20:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 70)
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r2.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 71)
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 72)
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r5.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 73)
    XMLVM_EXIT_METHOD()
    return;
    label27:;
    _r0 = _r4;
    goto label5;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder___INIT___]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 75)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 76)
    _r0.i = 16;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 77)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder___INIT____int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder___INIT____int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 79)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 80)
    if (_r2.i >= 0) goto label11;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 81)

    
    // Red class access removed: java.lang.NegativeArraySizeException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.NegativeArraySizeException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label11:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 83)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 84)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 86)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 87)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 88)
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r2.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 89)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + 16;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 90)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    java_lang_String_getChars___int_int_char_1ARRAY_int(_r4.o, _r2.i, _r0.i, _r1.o, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 91)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_enlargeBuffer___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_enlargeBuffer___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "enlargeBuffer", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 94)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r0.i >> 1;
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r0.i = _r0.i + _r1.i;
    _r0.i = _r0.i + 2;
    if (_r5.i <= _r0.i) goto label15;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 95)
    _r0 = _r5;
    label15:;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 96)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r3.i, _r0.o, _r3.i, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 97)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 98)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 99)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_appendNull__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_appendNull__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "appendNull", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 108;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 102)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + 4;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 103)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i <= _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 104)
    XMLVM_CHECK_NPE(4)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r4.o, _r0.i);
    label14:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 106)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r2.i;
    _r2.i = 110;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 107)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r2.i;
    _r2.i = 117;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 108)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 109)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 110)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_append0___char_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_append0___char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "append0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 113)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r0.i = _r0.i + _r1.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 114)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i <= _r1.i) goto label12;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 115)
    XMLVM_CHECK_NPE(5)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r5.o, _r0.i);
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 117)
    _r1.i = 0;
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r6.o, _r1.i, _r2.o, _r3.i, _r4.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 118)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 119)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_append0___char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_append0___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "append0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r3.o = me;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 123)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r5.i > _r0.i) goto label5;
    if (_r5.i >= 0) goto label17;
    label5:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 125)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.12"
    _r1.o = xmlvm_create_java_string_from_pool(61);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r1.o, _r5.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label17:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 127)
    if (_r6.i < 0) goto label23;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    _r0.i = _r0.i - _r5.i;
    if (_r0.i >= _r6.i) goto label35;
    label23:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 129)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.18"
    _r1.o = xmlvm_create_java_string_from_pool(62);
    _r1.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String_int(_r1.o, _r6.i);

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label35:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 132)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + _r6.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 133)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i <= _r1.i) goto label46;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 134)
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r3.o, _r0.i);
    label46:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 136)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r5.i, _r1.o, _r2.i, _r6.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 137)
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 138)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_append0___char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_append0___char]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "append0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 141)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i != _r1.i) goto label14;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 142)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r3.o, _r0.i);
    label14:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 144)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r2.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 145)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_append0___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_append0___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "append0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 148)
    if (_r6.o != JAVA_NULL) goto label6;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 149)
    XMLVM_CHECK_NPE(5)
    java_lang_AbstractStringBuilder_appendNull__(_r5.o);
    label5:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 159)
    XMLVM_EXIT_METHOD()
    return;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 152)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(6)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r6.o)->tib->vtable[8])(_r6.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 153)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = _r1.i + _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 154)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r1.i <= _r2.i) goto label21;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 155)
    XMLVM_CHECK_NPE(5)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r5.o, _r1.i);
    label21:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 157)
    _r2.i = 0;
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(5)
    _r4.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(6)
    java_lang_String_getChars___int_int_char_1ARRAY_int(_r6.o, _r2.i, _r0.i, _r3.o, _r4.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 158)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_ = _r1.i;
    goto label5;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_append0___java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_append0___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "append0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 162)
    if (_r3.o != JAVA_NULL) goto label34;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 163)
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label4:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 165)
    if (_r4.i < 0) goto label16;
    if (_r5.i < 0) goto label16;
    if (_r4.i > _r5.i) goto label16;
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r0.o);
    if (_r5.i <= _r1.i) goto label22;
    label16:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 166)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label22:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 169)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int])(_r0.o, _r4.i, _r5.i);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r0.o);
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_append0___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 170)
    XMLVM_EXIT_METHOD()
    return;
    label34:;
    _r0 = _r3;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_capacity__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_capacity__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "capacity", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 180)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_AbstractStringBuilder_charAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_charAt___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "charAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 194)
    if (_r2.i < 0) goto label6;
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i < _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 195)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 197)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_delete0___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_delete0___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "delete0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.i = n1;
    _r7.i = n2;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 201)
    if (_r6.i < 0) goto label58;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 202)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r7.i <= _r0.i) goto label64;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 203)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    label9:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 205)
    if (_r0.i != _r6.i) goto label12;
    label11:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 222)
    XMLVM_EXIT_METHOD()
    return;
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 208)
    if (_r0.i <= _r6.i) goto label58;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 209)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = _r1.i - _r0.i;
    if (_r1.i < 0) goto label30;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 210)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 211)
    XMLVM_CHECK_NPE(5)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r2.i != 0) goto label38;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 212)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r0.i, _r3.o, _r6.i, _r1.i);
    label30:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 221)
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r6.i;
    _r0.i = _r1.i - _r0.i;
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    goto label11;
    label38:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 214)
    XMLVM_CHECK_NPE(5)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    XMLVM_CLASS_INIT(char)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 215)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r4.i, _r2.o, _r4.i, _r6.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 216)
    XMLVM_CHECK_NPE(5)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r0.i, _r2.o, _r6.i, _r1.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 217)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_ = _r2.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 218)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r4.i;
    goto label30;
    label58:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 225)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label64:;
    _r0 = _r7;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_deleteCharAt0___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_deleteCharAt0___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "deleteCharAt0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.i = n1;
    _r5.i = 1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 229)
    if (_r7.i < 0) goto label8;
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r7.i < _r0.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 230)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 232)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r7.i;
    _r0.i = _r0.i - _r5.i;
    if (_r0.i <= 0) goto label33;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 233)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 234)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r1.i != 0) goto label39;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 235)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r7.i + 1;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r2.i, _r3.o, _r7.i, _r0.i);
    label33:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 246)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r5.i;
    XMLVM_CHECK_NPE(6)
    ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 247)
    XMLVM_EXIT_METHOD()
    return;
    label39:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 237)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    XMLVM_CLASS_INIT(char)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_char, _r1.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 238)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r4.i, _r1.o, _r4.i, _r7.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 240)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r3.i = _r7.i + 1;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r2.o, _r3.i, _r1.o, _r7.i, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 242)
    XMLVM_CHECK_NPE(6)
    ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.value_ = _r1.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 243)
    XMLVM_CHECK_NPE(6)
    ((java_lang_AbstractStringBuilder*) _r6.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r4.i;
    goto label33;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_ensureCapacity___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_ensureCapacity___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "ensureCapacity", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 262)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r2.i <= _r0.i) goto label17;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 263)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r0.i = _r0.i << 1;
    _r0.i = _r0.i + 2;
    if (_r0.i <= _r2.i) goto label18;
    label14:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 264)
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r1.o, _r0.i);
    label17:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 266)
    XMLVM_EXIT_METHOD()
    return;
    label18:;
    _r0 = _r2;
    goto label14;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_getChars___int_int_char_1ARRAY_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_getChars___int_int_char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "getChars", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r2.o = me;
    _r3.i = n1;
    _r4.i = n2;
    _r5.o = n3;
    _r6.i = n4;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 288)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r3.i > _r0.i) goto label10;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i > _r0.i) goto label10;
    if (_r3.i <= _r4.i) goto label16;
    label10:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 289)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label16:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 291)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = _r4.i - _r3.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r3.i, _r5.o, _r6.i, _r1.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 292)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "insert0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.i = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 295)
    if (_r4.i < 0) goto label6;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i <= _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 296)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 298)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r0.i == 0) goto label32;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 299)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_move___int_int(_r3.o, _r0.i, _r4.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 300)
    _r0.i = 0;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r5.o, _r0.i, _r1.o, _r4.i, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 301)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    label32:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 303)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_insert0___int_char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "insert0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r3.o = me;
    _r4.i = n1;
    _r5.o = n2;
    _r6.i = n3;
    _r7.i = n4;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 306)
    if (_r4.i < 0) goto label72;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i > _r0.i) goto label72;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 308)
    if (_r6.i < 0) goto label30;
    if (_r7.i < 0) goto label30;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    _r0.i = _r0.i - _r6.i;
    if (_r7.i > _r0.i) goto label30;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 309)
    if (_r7.i == 0) goto label29;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 310)
    XMLVM_CHECK_NPE(3)
    java_lang_AbstractStringBuilder_move___int_int(_r3.o, _r7.i, _r4.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 311)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r5.o, _r6.i, _r0.o, _r4.i, _r7.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 312)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + _r7.i;
    XMLVM_CHECK_NPE(3)
    ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    label29:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 314)
    XMLVM_EXIT_METHOD()
    return;
    label30:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 316)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r1.o = __NEW_java_lang_StringBuilder();
    // "offset "
    _r2.o = xmlvm_create_java_string_from_pool(64);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r6.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 317)
    // ", length "
    _r2.o = xmlvm_create_java_string_from_pool(65);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r7.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 318)
    // ", char[].length "
    _r2.o = xmlvm_create_java_string_from_pool(66);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___int(_r1.o, _r2.i);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label72:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 320)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_insert0___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_insert0___int_char]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "insert0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 324)
    if (_r2.i < 0) goto label6;
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i <= _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 326)

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.ArrayIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 328)
    _r0.i = 1;
    XMLVM_CHECK_NPE(1)
    java_lang_AbstractStringBuilder_move___int_int(_r1.o, _r0.i, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 329)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 330)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 331)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_insert0___int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_insert0___int_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "insert0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.i = n1;
    _r6.o = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 334)
    if (_r5.i < 0) goto label31;
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r5.i > _r0.i) goto label31;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 335)
    if (_r6.o != JAVA_NULL) goto label37;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 336)
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label10:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 338)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 339)
    if (_r1.i == 0) goto label30;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 340)
    XMLVM_CHECK_NPE(4)
    java_lang_AbstractStringBuilder_move___int_int(_r4.o, _r1.i, _r5.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 341)
    _r2.i = 0;
    XMLVM_CHECK_NPE(4)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(0)
    java_lang_String_getChars___int_int_char_1ARRAY_int(_r0.o, _r2.i, _r1.i, _r3.o, _r5.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 342)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + _r1.i;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    label30:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 347)
    XMLVM_EXIT_METHOD()
    return;
    label31:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 345)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label37:;
    _r0 = _r6;
    goto label10;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_insert0___int_java_lang_CharSequence_int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_insert0___int_java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "insert0", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r2.o = me;
    _r3.i = n1;
    _r4.o = n2;
    _r5.i = n3;
    _r6.i = n4;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 350)
    if (_r4.o != JAVA_NULL) goto label40;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 351)
    // "null"
    _r0.o = xmlvm_create_java_string_from_pool(63);
    label4:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 353)
    if (_r3.i < 0) goto label22;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r3.i > _r1.i) goto label22;
    if (_r5.i < 0) goto label22;
    if (_r6.i < 0) goto label22;
    if (_r5.i > _r6.i) goto label22;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 354)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r0.o);
    if (_r6.i <= _r1.i) goto label28;
    label22:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 355)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 357)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT, JAVA_INT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_subSequence___int_int])(_r0.o, _r5.i, _r6.i);
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_toString__])(_r0.o);
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r2.o, _r3.i, _r0.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 358)
    XMLVM_EXIT_METHOD()
    return;
    label40:;
    _r0 = _r4;
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_length__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_length__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "length", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 366)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_move___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_move___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "move", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r5.o = me;
    _r6.i = n1;
    _r7.i = n2;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 371)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(5)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r1.i;
    if (_r0.i < _r6.i) goto label51;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 372)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r0.i != 0) goto label26;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 373)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r7.i + _r6.i;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 374)
    _r3.i = _r3.i - _r7.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r0.o, _r7.i, _r1.o, _r2.i, _r3.i);
    label25:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 389)
    XMLVM_EXIT_METHOD()
    return;
    label26:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 377)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    label29:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 383)
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 384)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r4.i, _r0.o, _r4.i, _r7.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 386)
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r7.i + _r6.i;
    XMLVM_CHECK_NPE(5)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r3.i = _r3.i - _r7.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r7.i, _r0.o, _r2.i, _r3.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 387)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 388)
    XMLVM_CHECK_NPE(5)
    ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r4.i;
    goto label25;
    label51:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 379)
    XMLVM_CHECK_NPE(5)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i + _r6.i;
    XMLVM_CHECK_NPE(5)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r5.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r1.i = _r1.i << 1;
    _r1.i = _r1.i + 2;
    if (_r0.i > _r1.i) goto label29;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 380)
    _r0 = _r1;
    goto label29;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_replace0___int_int_java_lang_String(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_replace0___int_int_java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "replace0", "?")
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
    XMLVMElem _r11;
    _r8.o = me;
    _r9.i = n1;
    _r10.i = n2;
    _r11.o = n3;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 392)
    if (_r9.i < 0) goto label110;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 393)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r10.i <= _r0.i) goto label116;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 394)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_;
    label9:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 396)
    if (_r0.i <= _r9.i) goto label96;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 397)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(11)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r11.o)->tib->vtable[8])(_r11.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 398)
    _r2.i = _r0.i - _r9.i;
    _r2.i = _r2.i - _r1.i;
    if (_r2.i <= 0) goto label72;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 399)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 400)
    XMLVM_CHECK_NPE(8)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r3.i != 0) goto label47;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 402)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 403)
    _r5.i = _r9.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    _r6.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r6.i = _r6.i - _r0.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r3.o, _r0.i, _r4.o, _r5.i, _r6.i);
    label36:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 420)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(11)
    java_lang_String_getChars___int_int_char_1ARRAY_int(_r11.o, _r7.i, _r1.i, _r0.o, _r9.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 421)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r2.i;
    XMLVM_CHECK_NPE(8)
    ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_ = _r0.i;
    label46:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 429)
    XMLVM_EXIT_METHOD()
    return;
    label47:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 405)
    XMLVM_CHECK_NPE(8)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_CLASS_INIT(char)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_char, _r3.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 406)
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r7.i, _r3.o, _r7.i, _r9.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 408)
    XMLVM_CHECK_NPE(8)
    _r4.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 409)
    _r5.i = _r9.i + _r1.i;
    XMLVM_CHECK_NPE(8)
    _r6.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r6.i = _r6.i - _r0.i;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r4.o, _r0.i, _r3.o, _r5.i, _r6.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 410)
    XMLVM_CHECK_NPE(8)
    ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_ = _r3.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 411)
    XMLVM_CHECK_NPE(8)
    ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r7.i;
    goto label36;
    label72:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 413)
    if (_r2.i >= 0) goto label79;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 415)
    _r3.i = -_r2.i;
    XMLVM_CHECK_NPE(8)
    java_lang_AbstractStringBuilder_move___int_int(_r8.o, _r3.i, _r0.i);
    goto label36;
    label79:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 416)
    XMLVM_CHECK_NPE(8)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r0.i == 0) goto label36;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 417)
    XMLVM_CHECK_NPE(8)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_;
    //char_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->tib->vtable[0])(_r0.o);
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(8)
    ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 418)
    XMLVM_CHECK_NPE(8)
    ((java_lang_AbstractStringBuilder*) _r8.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r7.i;
    goto label36;
    label96:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 424)
    if (_r9.i != _r0.i) goto label110;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 425)
    if (_r11.o != JAVA_NULL) goto label106;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 426)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label106:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 428)
    XMLVM_CHECK_NPE(8)
    java_lang_AbstractStringBuilder_insert0___int_java_lang_String(_r8.o, _r9.i, _r11.o);
    goto label46;
    label110:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 432)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label116:;
    _r0 = _r10;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_reverse0__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_reverse0__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "reverse0", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    _r14.o = me;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 436)
    XMLVM_CHECK_NPE(14)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = 2;
    if (_r0.i >= _r1.i) goto label6;
    label5:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 510)
    XMLVM_EXIT_METHOD()
    return;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 439)
    XMLVM_CHECK_NPE(14)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r0.i != 0) goto label227;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 440)
    XMLVM_CHECK_NPE(14)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = 1;
    _r0.i = _r0.i - _r1.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 441)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = 0;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 442)
    XMLVM_CHECK_NPE(14)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 443)
    _r3.i = 1;
    _r4.i = 1;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 444)
    XMLVM_CHECK_NPE(14)
    _r6.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r6.i = _r6.i / 2;
    _r11 = _r5;
    _r5 = _r0;
    _r0 = _r11;
    _r12 = _r3;
    _r3 = _r2;
    _r2 = _r12;
    _r13 = _r1;
    _r1 = _r4;
    _r4 = _r13;
    label39:;
    if (_r0.i < _r6.i) goto label60;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 491)
    XMLVM_CHECK_NPE(14)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i & 1;
    _r6.i = 1;
    if (_r0.i != _r6.i) goto label5;
    if (_r2.i == 0) goto label52;
    if (_r1.i != 0) goto label5;
    label52:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 492)
    XMLVM_CHECK_NPE(14)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    if (_r2.i == 0) goto label224;
    _r1 = _r3;
    label57:;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r1.i;
    goto label5;
    label60:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 445)
    XMLVM_CHECK_NPE(14)
    _r7.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r8.i = _r0.i + 1;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r8.i);
    _r7.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 446)
    XMLVM_CHECK_NPE(14)
    _r8.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r9.i = 1;
    _r9.i = _r5.i - _r9.i;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r9.i);
    _r8.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 447)
    if (_r2.i == 0) goto label180;
    _r2.i = 56320;
    if (_r7.i < _r2.i) goto label180;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 448)
    _r2.i = 57343;
    if (_r7.i > _r2.i) goto label180;
    _r2.i = 55296;
    if (_r4.i < _r2.i) goto label180;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 449)
    _r2.i = 56319;
    if (_r4.i > _r2.i) goto label180;
    _r2.i = 1;
    label96:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 450)
    if (_r2.i == 0) goto label103;
    XMLVM_CHECK_NPE(14)
    _r9.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r10.i = 3;
    if (_r9.i < _r10.i) goto label5;
    label103:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 453)
    if (_r1.i == 0) goto label182;
    _r1.i = 55296;
    if (_r8.i < _r1.i) goto label182;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 454)
    _r1.i = 56319;
    if (_r8.i > _r1.i) goto label182;
    _r1.i = 56320;
    if (_r3.i < _r1.i) goto label182;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 455)
    _r1.i = 57343;
    if (_r3.i > _r1.i) goto label182;
    _r1.i = 1;
    label126:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 456)
    _r9.i = 1;
    if (_r2.i != _r1.i) goto label197;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 457)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 458)
    if (_r2.i == 0) goto label184;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 460)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r7.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 461)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = 1;
    _r2.i = _r5.i - _r2.i;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r4.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 462)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r8.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 463)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 464)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r0.i + 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 465)
    XMLVM_CHECK_NPE(14)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r3.i = 2;
    _r3.i = _r5.i - _r3.i;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 466)
    _r0.i = _r0.i + 1;
    _r3.i = _r5.i + -1;
    _r4 = _r1;
    _r5 = _r3;
    _r3 = _r2;
    _r1 = _r9;
    _r2 = _r9;
    label174:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 467)
    _r0.i = _r0.i + 1;
    _r5.i = _r5.i + -1;
    goto label39;
    label180:;
    _r2.i = 0;
    goto label96;
    label182:;
    _r1.i = 0;
    goto label126;
    label184:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 470)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r4.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 471)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    _r1 = _r9;
    _r2 = _r9;
    _r3 = _r8;
    _r4 = _r7;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 473)
    goto label174;
    label197:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 476)
    if (_r2.i == 0) goto label212;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 478)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r7.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 479)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 481)
    _r1.i = 0;
    _r2 = _r1;
    _r3 = _r8;
    _r1 = _r9;
    goto label174;
    label212:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 484)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r4.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 485)
    XMLVM_CHECK_NPE(14)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i] = _r8.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 487)
    _r1.i = 0;
    _r2 = _r9;
    _r4 = _r7;
    goto label174;
    label224:;
    _r1 = _r4;
    goto label57;
    label227:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 495)
    XMLVM_CHECK_NPE(14)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 496)
    _r1.i = 0;
    XMLVM_CHECK_NPE(14)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r11 = _r2;
    _r2 = _r1;
    _r1 = _r11;
    label238:;
    XMLVM_CHECK_NPE(14)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i < _r3.i) goto label249;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 507)
    XMLVM_CHECK_NPE(14)
    ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 508)
    _r0.i = 0;
    XMLVM_CHECK_NPE(14)
    ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r0.i;
    goto label5;
    label249:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 497)
    XMLVM_CHECK_NPE(14)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 498)
    _r4.i = _r2.i + 1;
    XMLVM_CHECK_NPE(14)
    _r5.i = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i >= _r5.i) goto label291;
    _r4.i = 55296;
    if (_r3.i < _r4.i) goto label291;
    _r4.i = 56319;
    if (_r3.i > _r4.i) goto label291;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 499)
    XMLVM_CHECK_NPE(14)
    _r4.o = ((java_lang_AbstractStringBuilder*) _r14.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r5.i = _r2.i + 1;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 500)
    _r5.i = 56320;
    if (_r4.i < _r5.i) goto label291;
    _r5.i = 57343;
    if (_r4.i > _r5.i) goto label291;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 501)
    _r1.i = _r1.i + -1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 502)
    _r2.i = _r2.i + 1;
    label291:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 505)
    _r1.i = _r1.i + -1;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r2.i = _r2.i + 1;
    goto label238;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_setCharAt___int_char(JAVA_OBJECT me, JAVA_INT n1, JAVA_CHAR n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_setCharAt___int_char]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "setCharAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 524)
    if (_r2.i < 0) goto label6;
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i < _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 525)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 527)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r0.i == 0) goto label29;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 528)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    //char_1ARRAY_clone__[0]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->tib->vtable[0])(_r0.o);
    _r0.o = _r0.o;
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 529)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r0.i;
    label29:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 531)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 532)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_setLength___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_setLength___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "setLength", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.o = me;
    _r5.i = n1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 546)
    if (_r5.i >= 0) goto label9;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 547)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label9:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 549)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r5.i <= _r0.i) goto label20;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 550)
    XMLVM_CHECK_NPE(4)
    java_lang_AbstractStringBuilder_enlargeBuffer___int(_r4.o, _r5.i);
    label17:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 563)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_ = _r5.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 564)
    XMLVM_EXIT_METHOD()
    return;
    label20:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 552)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.shared_;
    if (_r0.i == 0) goto label41;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 553)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 554)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r3.i, _r0.o, _r3.i, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 555)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 556)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r3.i;
    goto label17;
    label41:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 558)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r0.i >= _r5.i) goto label17;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 559)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;

    
    // Red class access removed: java.util.Arrays::fill
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_AbstractStringBuilder_substring___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_substring___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "substring", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 578)
    if (_r4.i < 0) goto label24;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i > _r0.i) goto label24;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 579)
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i != _r0.i) goto label13;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 580)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 584)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label13:;
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r2.i = _r2.i - _r4.i;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r1.o, _r4.i, _r2.i);
    goto label12;
    label24:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 586)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_AbstractStringBuilder_substring___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_substring___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "substring", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.i = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 603)
    if (_r4.i < 0) goto label23;
    if (_r4.i > _r5.i) goto label23;
    XMLVM_CHECK_NPE(3)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r5.i > _r0.i) goto label23;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 604)
    if (_r4.i != _r5.i) goto label13;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 605)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 609)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label13:;
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r2.i = _r5.i - _r4.i;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r1.o, _r4.i, _r2.i);
    goto label12;
    label23:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 611)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_AbstractStringBuilder_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_toString__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 621)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r0.i != 0) goto label8;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 622)
    // ""
    _r0.o = xmlvm_create_java_string_from_pool(21);
    label7:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 631)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label8:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 625)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = _r0.i - _r1.i;
    _r1.i = 256;
    if (_r0.i >= _r1.i) goto label28;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 626)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 627)
    _r1.i = 16;
    if (_r0.i < _r1.i) goto label38;
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = _r1.i >> 1;
    if (_r0.i < _r1.i) goto label38;
    label28:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 628)
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____char_1ARRAY_int_int(_r0.o, _r1.o, _r3.i, _r2.i);
    goto label7;
    label38:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 630)
    _r0.i = 1;
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r0.i;
    _r0.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(4)
    _r2.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(0)
    java_lang_String___INIT____int_int_char_1ARRAY(_r0.o, _r3.i, _r1.i, _r2.o);
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_AbstractStringBuilder_subSequence___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_subSequence___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "subSequence", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 649)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_AbstractStringBuilder_substring___int_int(_r1.o, _r2.i, _r3.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_indexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_indexOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "indexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 664)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_AbstractStringBuilder_indexOf___java_lang_String_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_indexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_indexOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "indexOf", "?")
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
    XMLVMElem _r11;
    _r9.o = me;
    _r10.o = n1;
    _r11.i = n2;
    _r8.i = -1;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 681)
    if (_r11.i >= 0) goto label81;
    _r0 = _r7;
    label5:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 682)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 684)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(10)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r10.o)->tib->vtable[8])(_r10.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 685)
    if (_r1.i <= 0) goto label72;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 686)
    _r2.i = _r1.i + _r0.i;
    XMLVM_CHECK_NPE(9)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i <= _r3.i) goto label19;
    _r0 = _r8;
    label18:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 687)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 713)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label19:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 690)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(10)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r10.o)->tib->vtable[6])(_r10.o, _r7.i);
    label23:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 694)
    XMLVM_CHECK_NPE(9)
    _r3.i = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r0.i < _r3.i) goto label38;
    _r3 = _r7;
    label28:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 700)
    if (_r3.i == 0) goto label36;
    _r3.i = _r1.i + _r0.i;
    XMLVM_CHECK_NPE(9)
    _r4.i = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r3.i <= _r4.i) goto label49;
    label36:;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 701)
    goto label18;
    label38:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 695)
    XMLVM_CHECK_NPE(9)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    if (_r3.i != _r2.i) goto label46;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 696)
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 697)
    goto label28;
    label46:;
    _r0.i = _r0.i + 1;
    goto label23;
    label49:;
    _r3 = _r7;
    _r4 = _r0;
    label51:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 703)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 704)
    _r3.i = _r3.i + 1;
    if (_r3.i >= _r1.i) goto label67;
    XMLVM_CHECK_NPE(9)
    _r5.o = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r4.i = _r4.i + 1;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r4.i);
    _r5.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(10)
    _r6.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r10.o)->tib->vtable[6])(_r10.o, _r3.i);
    if (_r5.i == _r6.i) goto label51;
    label67:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 707)
    if (_r3.i == _r1.i) goto label18;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 710)
    _r0.i = _r0.i + 1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 691)
    goto label23;
    label72:;
    XMLVM_CHECK_NPE(9)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r0.i < _r1.i) goto label18;
    if (_r0.i == 0) goto label18;
    XMLVM_CHECK_NPE(9)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r9.o)->fields.java_lang_AbstractStringBuilder.count_;
    goto label18;
    label81:;
    _r0 = _r11;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "lastIndexOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 730)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String_int(_r1.o, _r2.o, _r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_lastIndexOf___java_lang_String_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "lastIndexOf", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    _r10.o = me;
    _r11.o = n1;
    _r12.i = n2;
    _r9.i = 1;
    _r8.i = -1;
    _r7.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 749)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(11)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r11.o)->tib->vtable[8])(_r11.o);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 750)
    XMLVM_CHECK_NPE(10)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r0.i > _r1.i) goto label78;
    if (_r12.i < 0) goto label78;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 751)
    if (_r0.i <= 0) goto label69;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 752)
    XMLVM_CHECK_NPE(10)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = _r1.i - _r0.i;
    if (_r12.i <= _r1.i) goto label80;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 753)
    XMLVM_CHECK_NPE(10)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r1.i = _r1.i - _r0.i;
    label23:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 757)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(11)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r11.o)->tib->vtable[6])(_r11.o, _r7.i);
    label27:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 761)
    if (_r1.i >= 0) goto label34;
    _r3 = _r7;
    label30:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 767)
    if (_r3.i != 0) goto label45;
    _r0 = _r8;
    label33:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 768)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 783)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label34:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 762)
    XMLVM_CHECK_NPE(10)
    _r3.o = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r3.i != _r2.i) goto label42;
    _r3 = _r9;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 764)
    goto label30;
    label42:;
    _r1.i = _r1.i + -1;
    goto label27;
    label45:;
    _r3 = _r7;
    _r4 = _r1;
    label47:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 770)
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 771)
    _r3.i = _r3.i + 1;
    if (_r3.i >= _r0.i) goto label63;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 772)
    XMLVM_CHECK_NPE(10)
    _r5.o = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r4.i = _r4.i + 1;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r4.i);
    _r5.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(11)
    _r6.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r11.o)->tib->vtable[6])(_r11.o, _r3.i);
    if (_r5.i == _r6.i) goto label47;
    label63:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 775)
    if (_r3.i != _r0.i) goto label67;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 776)
    goto label33;
    label67:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 778)
    _r1.i = _r1.i - _r9.i;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 758)
    goto label27;
    label69:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 781)
    XMLVM_CHECK_NPE(10)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r12.i >= _r0.i) goto label75;
    _r0 = _r12;
    goto label33;
    label75:;
    XMLVM_CHECK_NPE(10)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r10.o)->fields.java_lang_AbstractStringBuilder.count_;
    goto label33;
    label78:;
    _r0 = _r8;
    goto label33;
    label80:;
    _r1 = _r12;
    goto label23;
    //XMLVM_END_WRAPPER
}

void java_lang_AbstractStringBuilder_trimToSize__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_trimToSize__]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "trimToSize", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 793)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i >= _r1.i) goto label23;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 794)
    XMLVM_CHECK_NPE(4)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 795)
    XMLVM_CHECK_NPE(4)
    _r1.o = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(4)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.count_;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r1.o, _r3.i, _r0.o, _r3.i, _r2.i);
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 796)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.value_ = _r0.o;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 797)
    XMLVM_CHECK_NPE(4)
    ((java_lang_AbstractStringBuilder*) _r4.o)->fields.java_lang_AbstractStringBuilder.shared_ = _r3.i;
    label23:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 799)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_codePointAt___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_codePointAt___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "codePointAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 815)
    if (_r3.i < 0) goto label6;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r3.i < _r0.i) goto label12;
    label6:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 816)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 818)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_CHECK_NPE(2)
    _r1.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = java_lang_Character_codePointAt___char_1ARRAY_int_int(_r0.o, _r3.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_codePointBefore___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_codePointBefore___int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "codePointBefore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 835)
    _r0.i = 1;
    if (_r2.i < _r0.i) goto label7;
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r2.i <= _r0.i) goto label13;
    label7:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 836)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label13:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 838)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r1.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r0.i = java_lang_Character_codePointBefore___char_1ARRAY_int(_r0.o, _r2.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_codePointCount___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_codePointCount___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "codePointCount", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.i = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 859)
    if (_r3.i < 0) goto label8;
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.count_;
    if (_r4.i > _r0.i) goto label8;
    if (_r3.i <= _r4.i) goto label14;
    label8:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 860)

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.StringIndexOutOfBoundsException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 862)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r2.o)->fields.java_lang_AbstractStringBuilder.value_;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 863)
    _r1.i = _r4.i - _r3.i;
    _r0.i = java_lang_Character_codePointCount___char_1ARRAY_int_int(_r0.o, _r3.i, _r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_AbstractStringBuilder_offsetByCodePoints___int_int(JAVA_OBJECT me, JAVA_INT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_AbstractStringBuilder_offsetByCodePoints___int_int]
    XMLVM_ENTER_METHOD("java.lang.AbstractStringBuilder", "offsetByCodePoints", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.i = n1;
    _r5.i = n2;
    XMLVM_SOURCE_POSITION("AbstractStringBuilder.java", 886)
    XMLVM_CHECK_NPE(3)
    _r0.o = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.value_;
    _r1.i = 0;
    XMLVM_CHECK_NPE(3)
    _r2.i = ((java_lang_AbstractStringBuilder*) _r3.o)->fields.java_lang_AbstractStringBuilder.count_;
    _r0.i = java_lang_Character_offsetByCodePoints___char_1ARRAY_int_int_int_int(_r0.o, _r1.i, _r2.i, _r4.i, _r5.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

