#include "xmlvm.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"

#include "java_util_StringTokenizer.h"

#define XMLVM_CURRENT_CLASS_NAME StringTokenizer
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_StringTokenizer

__TIB_DEFINITION_java_util_StringTokenizer __TIB_java_util_StringTokenizer = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_StringTokenizer, // classInitializer
    "java.util.StringTokenizer", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/util/Enumeration<Ljava/lang/Object;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_util_StringTokenizer), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_StringTokenizer;
JAVA_OBJECT __CLASS_java_util_StringTokenizer_1ARRAY;
JAVA_OBJECT __CLASS_java_util_StringTokenizer_2ARRAY;
JAVA_OBJECT __CLASS_java_util_StringTokenizer_3ARRAY;
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

void __INIT_java_util_StringTokenizer()
{
    staticInitializerLock(&__TIB_java_util_StringTokenizer);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_StringTokenizer.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_StringTokenizer.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_StringTokenizer);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_StringTokenizer.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_StringTokenizer.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_StringTokenizer.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.StringTokenizer")
        __INIT_IMPL_java_util_StringTokenizer();
    }
}

void __INIT_IMPL_java_util_StringTokenizer()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_util_StringTokenizer.newInstanceFunc = __NEW_INSTANCE_java_util_StringTokenizer;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_StringTokenizer.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_util_StringTokenizer.vtable[6] = (VTABLE_PTR) &java_util_StringTokenizer_hasMoreElements__;
    __TIB_java_util_StringTokenizer.vtable[7] = (VTABLE_PTR) &java_util_StringTokenizer_nextElement__;
    // Initialize interface information
    __TIB_java_util_StringTokenizer.numImplementedInterfaces = 1;
    __TIB_java_util_StringTokenizer.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_util_Enumeration)

    __TIB_java_util_StringTokenizer.implementedInterfaces[0][0] = &__TIB_java_util_Enumeration;
    // Initialize itable for this class
    __TIB_java_util_StringTokenizer.itableBegin = &__TIB_java_util_StringTokenizer.itable[0];
    __TIB_java_util_StringTokenizer.itable[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__] = __TIB_java_util_StringTokenizer.vtable[6];
    __TIB_java_util_StringTokenizer.itable[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__] = __TIB_java_util_StringTokenizer.vtable[7];


    __TIB_java_util_StringTokenizer.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_StringTokenizer.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_StringTokenizer.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_StringTokenizer.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_StringTokenizer.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_StringTokenizer.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_StringTokenizer.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_StringTokenizer.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_StringTokenizer = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_StringTokenizer);
    __TIB_java_util_StringTokenizer.clazz = __CLASS_java_util_StringTokenizer;
    __TIB_java_util_StringTokenizer.baseType = JAVA_NULL;
    __CLASS_java_util_StringTokenizer_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_StringTokenizer);
    __CLASS_java_util_StringTokenizer_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_StringTokenizer_1ARRAY);
    __CLASS_java_util_StringTokenizer_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_StringTokenizer_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_StringTokenizer]
    //XMLVM_END_WRAPPER

    __TIB_java_util_StringTokenizer.classInitialized = 1;
}

void __DELETE_java_util_StringTokenizer(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_StringTokenizer]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_StringTokenizer(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_StringTokenizer*) me)->fields.java_util_StringTokenizer.string_ = (java_lang_String*) JAVA_NULL;
    ((java_util_StringTokenizer*) me)->fields.java_util_StringTokenizer.delimiters_ = (java_lang_String*) JAVA_NULL;
    ((java_util_StringTokenizer*) me)->fields.java_util_StringTokenizer.returnDelimiters_ = 0;
    ((java_util_StringTokenizer*) me)->fields.java_util_StringTokenizer.position_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_StringTokenizer]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_StringTokenizer()
{    XMLVM_CLASS_INIT(java_util_StringTokenizer)
java_util_StringTokenizer* me = (java_util_StringTokenizer*) XMLVM_MALLOC(sizeof(java_util_StringTokenizer));
    me->tib = &__TIB_java_util_StringTokenizer;
    __INIT_INSTANCE_MEMBERS_java_util_StringTokenizer(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_StringTokenizer]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_StringTokenizer()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

void java_util_StringTokenizer___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer___INIT____java_lang_String]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 123)
    // " \011\012\015\014"
    _r0.o = xmlvm_create_java_string_from_pool(649);
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean(_r2.o, _r3.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 124)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_StringTokenizer___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer___INIT____java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 139)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean(_r1.o, _r2.o, _r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 140)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer___INIT____java_lang_String_java_lang_String_boolean]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    _r4.i = n3;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 156)
    XMLVM_CHECK_NPE(1)
    java_lang_Object___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 158)
    if (_r2.o == JAVA_NULL) goto label15;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 159)
    XMLVM_CHECK_NPE(1)
    ((java_util_StringTokenizer*) _r1.o)->fields.java_util_StringTokenizer.string_ = _r2.o;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 160)
    XMLVM_CHECK_NPE(1)
    ((java_util_StringTokenizer*) _r1.o)->fields.java_util_StringTokenizer.delimiters_ = _r3.o;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 161)
    XMLVM_CHECK_NPE(1)
    ((java_util_StringTokenizer*) _r1.o)->fields.java_util_StringTokenizer.returnDelimiters_ = _r4.i;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 162)
    _r0.i = 0;
    XMLVM_CHECK_NPE(1)
    ((java_util_StringTokenizer*) _r1.o)->fields.java_util_StringTokenizer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 165)
    XMLVM_EXIT_METHOD()
    return;
    label15:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 164)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_util_StringTokenizer_countTokens__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_countTokens__]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "countTokens", "?")
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
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 174)
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 176)
    XMLVM_CHECK_NPE(7)
    _r0.i = ((java_util_StringTokenizer*) _r7.o)->fields.java_util_StringTokenizer.position_;
    XMLVM_CHECK_NPE(7)
    _r1.o = ((java_util_StringTokenizer*) _r7.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    _r2 = _r6;
    _r3 = _r6;
    label11:;
    if (_r0.i < _r1.i) goto label18;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 188)
    if (_r2.i == 0) goto label49;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 189)
    _r0.i = _r3.i + 1;
    label17:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 190)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 177)
    XMLVM_CHECK_NPE(7)
    _r4.o = ((java_util_StringTokenizer*) _r7.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(7)
    _r5.o = ((java_util_StringTokenizer*) _r7.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(5)
    _r5.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r5.o)->tib->vtable[6])(_r5.o, _r0.i);
    XMLVM_CHECK_NPE(4)
    _r4.i = java_lang_String_indexOf___int_int(_r4.o, _r5.i, _r6.i);
    if (_r4.i < 0) goto label47;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 178)
    XMLVM_CHECK_NPE(7)
    _r4.i = ((java_util_StringTokenizer*) _r7.o)->fields.java_util_StringTokenizer.returnDelimiters_;
    if (_r4.i == 0) goto label38;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 179)
    _r3.i = _r3.i + 1;
    label38:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 180)
    if (_r2.i == 0) goto label44;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 181)
    _r2.i = _r3.i + 1;
    _r3 = _r2;
    _r2 = _r6;
    label44:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 182)
    _r0.i = _r0.i + 1;
    goto label11;
    label47:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 185)
    _r2.i = 1;
    goto label44;
    label49:;
    _r0 = _r3;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_StringTokenizer_hasMoreElements__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_hasMoreElements__]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "hasMoreElements", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 200)
    XMLVM_CHECK_NPE(1)
    _r0.i = java_util_StringTokenizer_hasMoreTokens__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_StringTokenizer_hasMoreTokens__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_hasMoreTokens__]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "hasMoreTokens", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    _r5.i = 1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 209)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    if (_r0.o != JAVA_NULL) goto label12;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 210)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 212)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 213)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    if (_r1.i >= _r0.i) goto label32;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 214)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.returnDelimiters_;
    if (_r1.i == 0) goto label28;
    _r0 = _r5;
    label27:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 215)
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 223)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label28:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 219)
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    label30:;
    if (_r1.i < _r0.i) goto label34;
    label32:;
    _r0 = _r4;
    goto label27;
    label34:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 220)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r1.i);
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_String_indexOf___int_int(_r2.o, _r3.i, _r4.i);
    _r3.i = -1;
    if (_r2.i != _r3.i) goto label51;
    _r0 = _r5;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 221)
    goto label27;
    label51:;
    _r1.i = _r1.i + 1;
    goto label30;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_StringTokenizer_nextElement__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_nextElement__]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "nextElement", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 235)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_StringTokenizer_nextToken__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_StringTokenizer_nextToken__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_nextToken__]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "nextToken", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r6.o = me;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 246)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    if (_r0.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 247)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label11:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 249)
    XMLVM_CHECK_NPE(6)
    _r0.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 250)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 252)
    if (_r0.i >= _r1.i) goto label178;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 253)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.returnDelimiters_;
    if (_r2.i == 0) goto label109;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 254)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r4.i);
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_String_indexOf___int_int(_r2.o, _r3.i, _r5.i);
    if (_r2.i < 0) goto label58;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 255)
    XMLVM_CHECK_NPE(6)
    _r0.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r1.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    _r2.i = _r1.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r2.i;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r0.o)->tib->vtable[6])(_r0.o, _r1.i);
    _r0.o = java_lang_String_valueOf___char(_r0.i);
    label57:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 269)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label58:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 256)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r2.i;
    label64:;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    if (_r2.i < _r1.i) goto label75;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 259)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int(_r1.o, _r0.i);
    goto label57;
    label75:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 257)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r4.i);
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_String_indexOf___int_int(_r2.o, _r3.i, _r5.i);
    if (_r2.i < 0) goto label100;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 258)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int_int(_r1.o, _r0.i, _r2.i);
    goto label57;
    label100:;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r2.i;
    goto label64;
    label107:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 263)
    _r0.i = _r0.i + 1;
    label109:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 262)
    if (_r0.i >= _r1.i) goto label125;
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r0.i);
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_String_indexOf___int_int(_r2.o, _r3.i, _r5.i);
    if (_r2.i >= 0) goto label107;
    label125:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 264)
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r0.i;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 265)
    if (_r0.i >= _r1.i) goto label178;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 266)
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r2.i;
    label135:;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    if (_r2.i < _r1.i) goto label146;
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int(_r1.o, _r0.i);
    goto label57;
    label146:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 267)
    XMLVM_CHECK_NPE(6)
    _r2.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.delimiters_;
    XMLVM_CHECK_NPE(6)
    _r3.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r4.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r4.i);
    XMLVM_CHECK_NPE(2)
    _r2.i = java_lang_String_indexOf___int_int(_r2.o, _r3.i, _r5.i);
    if (_r2.i < 0) goto label171;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 268)
    XMLVM_CHECK_NPE(6)
    _r1.o = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.string_;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int_int(_r1.o, _r0.i, _r2.i);
    goto label57;
    label171:;
    XMLVM_CHECK_NPE(6)
    _r2.i = ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_;
    _r2.i = _r2.i + 1;
    XMLVM_CHECK_NPE(6)
    ((java_util_StringTokenizer*) _r6.o)->fields.java_util_StringTokenizer.position_ = _r2.i;
    goto label135;
    label178:;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 272)

    
    // Red class access removed: java.util.NoSuchElementException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.NoSuchElementException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_StringTokenizer_nextToken___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_StringTokenizer_nextToken___java_lang_String]
    XMLVM_ENTER_METHOD("java.util.StringTokenizer", "nextToken", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 286)
    XMLVM_CHECK_NPE(1)
    ((java_util_StringTokenizer*) _r1.o)->fields.java_util_StringTokenizer.delimiters_ = _r2.o;
    XMLVM_SOURCE_POSITION("StringTokenizer.java", 287)
    XMLVM_CHECK_NPE(1)
    _r0.o = java_util_StringTokenizer_nextToken__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

