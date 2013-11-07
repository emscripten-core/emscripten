#include "xmlvm.h"
#include "java_io_IOException.h"
#include "java_io_OutputStream.h"
#include "java_io_PrintStream.h"
#include "java_lang_Character.h"
#include "java_lang_Class.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "java_lang_Throwable.h"
#include "java_nio_charset_Charset.h"
#include "java_security_AccessController.h"
#include "java_util_Collections.h"
#include "java_util_Enumeration.h"
#include "java_util_Iterator.h"
#include "java_util_Map_Entry.h"
#include "java_util_Set.h"
#include "org_apache_harmony_luni_internal_nls_Messages.h"
#include "org_apache_harmony_luni_util_PriviAction.h"

#include "java_util_Properties.h"

#define XMLVM_CURRENT_CLASS_NAME Properties
#define XMLVM_CURRENT_PKG_CLASS_NAME java_util_Properties

__TIB_DEFINITION_java_util_Properties __TIB_java_util_Properties = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_util_Properties, // classInitializer
    "java.util.Properties", // className
    "java.util", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/util/Hashtable<Ljava/lang/Object;Ljava/lang/Object;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_util_Hashtable, // extends
    sizeof(java_util_Properties), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_util_Properties;
JAVA_OBJECT __CLASS_java_util_Properties_1ARRAY;
JAVA_OBJECT __CLASS_java_util_Properties_2ARRAY;
JAVA_OBJECT __CLASS_java_util_Properties_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_util_Properties_serialVersionUID;
static JAVA_OBJECT _STATIC_java_util_Properties_PROP_DTD_NAME;
static JAVA_OBJECT _STATIC_java_util_Properties_PROP_DTD;
static JAVA_INT _STATIC_java_util_Properties_NONE;
static JAVA_INT _STATIC_java_util_Properties_SLASH;
static JAVA_INT _STATIC_java_util_Properties_UNICODE;
static JAVA_INT _STATIC_java_util_Properties_CONTINUE;
static JAVA_INT _STATIC_java_util_Properties_KEY_DONE;
static JAVA_INT _STATIC_java_util_Properties_IGNORE;
static JAVA_OBJECT _STATIC_java_util_Properties_lineSeparator;

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

void __INIT_java_util_Properties()
{
    staticInitializerLock(&__TIB_java_util_Properties);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_util_Properties.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_util_Properties.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_util_Properties);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_util_Properties.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_util_Properties.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_util_Properties.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.util.Properties")
        __INIT_IMPL_java_util_Properties();
    }
}

void __INIT_IMPL_java_util_Properties()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_util_Hashtable)
    __TIB_java_util_Properties.newInstanceFunc = __NEW_INSTANCE_java_util_Properties;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_util_Properties.vtable, __TIB_java_util_Hashtable.vtable, sizeof(__TIB_java_util_Hashtable.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_java_util_Properties.numImplementedInterfaces = 3;
    __TIB_java_util_Properties.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 3);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_util_Properties.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Cloneable)

    __TIB_java_util_Properties.implementedInterfaces[0][1] = &__TIB_java_lang_Cloneable;

    XMLVM_CLASS_INIT(java_util_Map)

    __TIB_java_util_Properties.implementedInterfaces[0][2] = &__TIB_java_util_Map;
    // Initialize itable for this class
    __TIB_java_util_Properties.itableBegin = &__TIB_java_util_Properties.itable[0];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_clear__] = __TIB_java_util_Properties.vtable[13];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_containsKey___java_lang_Object] = __TIB_java_util_Properties.vtable[14];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_containsValue___java_lang_Object] = __TIB_java_util_Properties.vtable[15];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_entrySet__] = __TIB_java_util_Properties.vtable[16];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_equals___java_lang_Object] = __TIB_java_util_Properties.vtable[1];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object] = __TIB_java_util_Properties.vtable[7];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_hashCode__] = __TIB_java_util_Properties.vtable[4];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_isEmpty__] = __TIB_java_util_Properties.vtable[8];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_keySet__] = __TIB_java_util_Properties.vtable[17];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_putAll___java_util_Map] = __TIB_java_util_Properties.vtable[18];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object] = __TIB_java_util_Properties.vtable[10];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_remove___java_lang_Object] = __TIB_java_util_Properties.vtable[11];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_size__] = __TIB_java_util_Properties.vtable[12];
    __TIB_java_util_Properties.itable[XMLVM_ITABLE_IDX_java_util_Map_values__] = __TIB_java_util_Properties.vtable[19];

    _STATIC_java_util_Properties_serialVersionUID = 4112578634029874840;
    _STATIC_java_util_Properties_PROP_DTD_NAME = (java_lang_String*) xmlvm_create_java_string_from_pool(102);
    _STATIC_java_util_Properties_PROP_DTD = (java_lang_String*) xmlvm_create_java_string_from_pool(103);
    _STATIC_java_util_Properties_NONE = 0;
    _STATIC_java_util_Properties_SLASH = 1;
    _STATIC_java_util_Properties_UNICODE = 2;
    _STATIC_java_util_Properties_CONTINUE = 3;
    _STATIC_java_util_Properties_KEY_DONE = 4;
    _STATIC_java_util_Properties_IGNORE = 5;
    _STATIC_java_util_Properties_lineSeparator = (java_lang_String*) JAVA_NULL;

    __TIB_java_util_Properties.declaredFields = &__field_reflection_data[0];
    __TIB_java_util_Properties.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_util_Properties.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_util_Properties.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_util_Properties.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_util_Properties.methodDispatcherFunc = method_dispatcher;
    __TIB_java_util_Properties.declaredMethods = &__method_reflection_data[0];
    __TIB_java_util_Properties.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_util_Properties = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_util_Properties);
    __TIB_java_util_Properties.clazz = __CLASS_java_util_Properties;
    __TIB_java_util_Properties.baseType = JAVA_NULL;
    __CLASS_java_util_Properties_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Properties);
    __CLASS_java_util_Properties_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Properties_1ARRAY);
    __CLASS_java_util_Properties_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_util_Properties_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_java_util_Properties]
    //XMLVM_END_WRAPPER

    __TIB_java_util_Properties.classInitialized = 1;
}

void __DELETE_java_util_Properties(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_util_Properties]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_util_Properties(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_util_Hashtable(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_util_Properties*) me)->fields.java_util_Properties.builder_ = (java_lang_Object*) JAVA_NULL;
    ((java_util_Properties*) me)->fields.java_util_Properties.defaults_ = (java_util_Properties*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_util_Properties]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_util_Properties()
{    XMLVM_CLASS_INIT(java_util_Properties)
java_util_Properties* me = (java_util_Properties*) XMLVM_MALLOC(sizeof(java_util_Properties));
    me->tib = &__TIB_java_util_Properties;
    __INIT_INSTANCE_MEMBERS_java_util_Properties(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_util_Properties]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_util_Properties()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_java_util_Properties();
    java_util_Properties___INIT___(me);
    return me;
}

JAVA_LONG java_util_Properties_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_serialVersionUID;
}

void java_util_Properties_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_serialVersionUID = v;
}

JAVA_OBJECT java_util_Properties_GET_PROP_DTD_NAME()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_PROP_DTD_NAME;
}

void java_util_Properties_PUT_PROP_DTD_NAME(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_PROP_DTD_NAME = v;
}

JAVA_OBJECT java_util_Properties_GET_PROP_DTD()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_PROP_DTD;
}

void java_util_Properties_PUT_PROP_DTD(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_PROP_DTD = v;
}

JAVA_INT java_util_Properties_GET_NONE()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_NONE;
}

void java_util_Properties_PUT_NONE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_NONE = v;
}

JAVA_INT java_util_Properties_GET_SLASH()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_SLASH;
}

void java_util_Properties_PUT_SLASH(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_SLASH = v;
}

JAVA_INT java_util_Properties_GET_UNICODE()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_UNICODE;
}

void java_util_Properties_PUT_UNICODE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_UNICODE = v;
}

JAVA_INT java_util_Properties_GET_CONTINUE()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_CONTINUE;
}

void java_util_Properties_PUT_CONTINUE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_CONTINUE = v;
}

JAVA_INT java_util_Properties_GET_KEY_DONE()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_KEY_DONE;
}

void java_util_Properties_PUT_KEY_DONE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_KEY_DONE = v;
}

JAVA_INT java_util_Properties_GET_IGNORE()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_IGNORE;
}

void java_util_Properties_PUT_IGNORE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_IGNORE = v;
}

JAVA_OBJECT java_util_Properties_GET_lineSeparator()
{
    XMLVM_CLASS_INIT(java_util_Properties)
    return _STATIC_java_util_Properties_lineSeparator;
}

void java_util_Properties_PUT_lineSeparator(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_util_Properties)
_STATIC_java_util_Properties_lineSeparator = v;
}

void java_util_Properties___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties___INIT___]
    XMLVM_ENTER_METHOD("java.util.Properties", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Properties.java", 90)
    XMLVM_CHECK_NPE(1)
    java_util_Hashtable___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 66)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    ((java_util_Properties*) _r1.o)->fields.java_util_Properties.builder_ = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 91)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Properties___INIT____java_util_Properties(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties___INIT____java_util_Properties]
    XMLVM_ENTER_METHOD("java.util.Properties", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 100)
    XMLVM_CHECK_NPE(1)
    java_util_Hashtable___INIT___(_r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 66)
    _r0.o = JAVA_NULL;
    XMLVM_CHECK_NPE(1)
    ((java_util_Properties*) _r1.o)->fields.java_util_Properties.builder_ = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 101)
    XMLVM_CHECK_NPE(1)
    ((java_util_Properties*) _r1.o)->fields.java_util_Properties.defaults_ = _r2.o;
    XMLVM_SOURCE_POSITION("Properties.java", 102)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean]
    XMLVM_ENTER_METHOD("java.util.Properties", "dumpString", "?")
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
    _r5.o = me;
    _r6.o = n1;
    _r7.o = n2;
    _r8.i = n3;
    _r9.i = n4;
    _r4.i = 32;
    XMLVM_SOURCE_POSITION("Properties.java", 106)
    _r0.i = 0;
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(7)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r7.o)->tib->vtable[8])(_r7.o);
    XMLVM_SOURCE_POSITION("Properties.java", 107)
    if (_r8.i != 0) goto label24;
    if (_r0.i >= _r1.i) goto label24;
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(7)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r7.o)->tib->vtable[6])(_r7.o, _r0.i);
    if (_r2.i != _r4.i) goto label24;
    XMLVM_SOURCE_POSITION("Properties.java", 108)
    // "\134 "
    _r2.o = xmlvm_create_java_string_from_pool(104);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___java_lang_String(_r6.o, _r2.o);
    XMLVM_SOURCE_POSITION("Properties.java", 109)
    _r0.i = _r0.i + 1;
    label24:;
    XMLVM_SOURCE_POSITION("Properties.java", 112)
    if (_r0.i < _r1.i) goto label27;
    XMLVM_SOURCE_POSITION("Properties.java", 142)
    XMLVM_EXIT_METHOD()
    return;
    label27:;
    XMLVM_SOURCE_POSITION("Properties.java", 113)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(7)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r7.o)->tib->vtable[6])(_r7.o, _r0.i);
    XMLVM_SOURCE_POSITION("Properties.java", 114)
    switch (_r2.i) {
    case 9: goto label63;
    case 10: goto label69;
    case 12: goto label75;
    case 13: goto label81;
    }
    XMLVM_SOURCE_POSITION("Properties.java", 128)
    // "\134#!=:"
    _r3.o = xmlvm_create_java_string_from_pool(105);
    XMLVM_CHECK_NPE(3)
    _r3.i = java_lang_String_indexOf___int(_r3.o, _r2.i);
    if (_r3.i >= 0) goto label46;
    if (_r8.i == 0) goto label51;
    if (_r2.i != _r4.i) goto label51;
    label46:;
    XMLVM_SOURCE_POSITION("Properties.java", 129)
    _r3.i = 92;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(6)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r6.o)->tib->vtable[6])(_r6.o, _r3.i);
    label51:;
    XMLVM_SOURCE_POSITION("Properties.java", 131)
    if (_r2.i < _r4.i) goto label87;
    _r3.i = 126;
    if (_r2.i > _r3.i) goto label87;
    XMLVM_SOURCE_POSITION("Properties.java", 132)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(6)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r6.o)->tib->vtable[6])(_r6.o, _r2.i);
    label60:;
    _r0.i = _r0.i + 1;
    goto label24;
    label63:;
    XMLVM_SOURCE_POSITION("Properties.java", 116)
    // "\134t"
    _r2.o = xmlvm_create_java_string_from_pool(106);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___java_lang_String(_r6.o, _r2.o);
    goto label60;
    label69:;
    XMLVM_SOURCE_POSITION("Properties.java", 119)
    // "\134n"
    _r2.o = xmlvm_create_java_string_from_pool(107);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___java_lang_String(_r6.o, _r2.o);
    goto label60;
    label75:;
    XMLVM_SOURCE_POSITION("Properties.java", 122)
    // "\134f"
    _r2.o = xmlvm_create_java_string_from_pool(108);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___java_lang_String(_r6.o, _r2.o);
    goto label60;
    label81:;
    XMLVM_SOURCE_POSITION("Properties.java", 125)
    // "\134r"
    _r2.o = xmlvm_create_java_string_from_pool(109);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___java_lang_String(_r6.o, _r2.o);
    goto label60;
    label87:;
    XMLVM_SOURCE_POSITION("Properties.java", 134)
    if (_r9.i == 0) goto label97;
    XMLVM_SOURCE_POSITION("Properties.java", 135)
    XMLVM_CHECK_NPE(5)
    _r2.o = java_util_Properties_toHexaDecimal___int(_r5.o, _r2.i);
    XMLVM_CHECK_NPE(6)
    java_lang_StringBuilder_append___char_1ARRAY(_r6.o, _r2.o);
    goto label60;
    label97:;
    XMLVM_SOURCE_POSITION("Properties.java", 137)
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(6)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r6.o)->tib->vtable[6])(_r6.o, _r2.i);
    goto label60;
    label102:;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_toHexaDecimal___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_toHexaDecimal___int]
    XMLVM_ENTER_METHOD("java.util.Properties", "toHexaDecimal", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r5.o = me;
    _r6.i = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 145)
    _r0.i = 6;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_CHAR[]){92, 117, 48, 48, 48, 48, });
    XMLVM_SOURCE_POSITION("Properties.java", 146)
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    _r2 = _r1;
    _r1 = _r6;
    label9:;
    XMLVM_SOURCE_POSITION("Properties.java", 148)
    _r3.i = _r1.i & 15;
    _r4.i = 9;
    if (_r3.i <= _r4.i) goto label30;
    XMLVM_SOURCE_POSITION("Properties.java", 149)
    XMLVM_SOURCE_POSITION("Properties.java", 150)
    _r4.i = 10;
    _r3.i = _r3.i - _r4.i;
    _r3.i = _r3.i + 65;
    label20:;
    XMLVM_SOURCE_POSITION("Properties.java", 154)
    _r2.i = _r2.i + -1;
    _r3.i = _r3.i & 0xffff;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i] = _r3.i;
    XMLVM_SOURCE_POSITION("Properties.java", 155)
    _r1.i = ((JAVA_UINT) _r1.i) >> (0x1f & ((JAVA_UINT) 4));
    if (_r1.i != 0) goto label9;
    XMLVM_SOURCE_POSITION("Properties.java", 156)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label30:;
    XMLVM_SOURCE_POSITION("Properties.java", 152)
    _r3.i = _r3.i + 48;
    goto label20;
    label34:;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_getProperty___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_getProperty___java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "getProperty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 169)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_Hashtable_get___java_lang_Object(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("Properties.java", 170)
    XMLVM_CLASS_INIT(java_lang_String)
    _r1.i = XMLVM_ISA(_r0.o, __CLASS_java_lang_String);
    if (_r1.i == 0) goto label23;
    _r0.o = _r0.o;
    label10:;
    XMLVM_SOURCE_POSITION("Properties.java", 171)
    if (_r0.o != JAVA_NULL) goto label22;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Properties*) _r2.o)->fields.java_util_Properties.defaults_;
    if (_r1.o == JAVA_NULL) goto label22;
    XMLVM_SOURCE_POSITION("Properties.java", 172)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Properties*) _r2.o)->fields.java_util_Properties.defaults_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_util_Properties_getProperty___java_lang_String(_r0.o, _r3.o);
    label22:;
    XMLVM_SOURCE_POSITION("Properties.java", 174)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label23:;
    _r0.o = JAVA_NULL;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_getProperty___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_getProperty___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "getProperty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r2.o = me;
    _r3.o = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 190)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_util_Hashtable_get___java_lang_Object(_r2.o, _r3.o);
    XMLVM_SOURCE_POSITION("Properties.java", 191)
    XMLVM_CLASS_INIT(java_lang_String)
    _r1.i = XMLVM_ISA(_r0.o, __CLASS_java_lang_String);
    if (_r1.i == 0) goto label26;
    _r0.o = _r0.o;
    label10:;
    XMLVM_SOURCE_POSITION("Properties.java", 192)
    if (_r0.o != JAVA_NULL) goto label22;
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_util_Properties*) _r2.o)->fields.java_util_Properties.defaults_;
    if (_r1.o == JAVA_NULL) goto label22;
    XMLVM_SOURCE_POSITION("Properties.java", 193)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_util_Properties*) _r2.o)->fields.java_util_Properties.defaults_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_util_Properties_getProperty___java_lang_String(_r0.o, _r3.o);
    label22:;
    XMLVM_SOURCE_POSITION("Properties.java", 195)
    if (_r0.o != JAVA_NULL) goto label25;
    _r0 = _r4;
    label25:;
    XMLVM_SOURCE_POSITION("Properties.java", 196)
    XMLVM_SOURCE_POSITION("Properties.java", 198)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label26:;
    _r0.o = JAVA_NULL;
    goto label10;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_list___java_io_PrintStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_list___java_io_PrintStream]
    XMLVM_ENTER_METHOD("java.util.Properties", "list", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("Properties.java", 211)
    if (_r7.o != JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Properties.java", 212)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label9:;
    XMLVM_SOURCE_POSITION("Properties.java", 214)
    _r2.o = __NEW_java_lang_StringBuilder();
    _r0.i = 80;
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("Properties.java", 215)
    XMLVM_CHECK_NPE(6)
    _r3.o = java_util_Properties_propertyNames__(_r6.o);
    label20:;
    XMLVM_SOURCE_POSITION("Properties.java", 216)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r3.o);
    if (_r0.i != 0) goto label27;
    XMLVM_SOURCE_POSITION("Properties.java", 235)
    XMLVM_EXIT_METHOD()
    return;
    label27:;
    XMLVM_SOURCE_POSITION("Properties.java", 217)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r3.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 218)
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 219)
    _r1.i = 61;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[6])(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 220)
    XMLVM_CHECK_NPE(6)
    _r1.o = java_util_Hashtable_get___java_lang_Object(_r6.o, _r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 221)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_util_Properties*) _r6.o)->fields.java_util_Properties.defaults_;
    label49:;
    XMLVM_SOURCE_POSITION("Properties.java", 222)
    if (_r1.o == JAVA_NULL) goto label84;
    XMLVM_SOURCE_POSITION("Properties.java", 226)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    _r4.i = 40;
    if (_r0.i <= _r4.i) goto label93;
    XMLVM_SOURCE_POSITION("Properties.java", 227)
    _r0.i = 37;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int_int(_r1.o, _r5.i, _r0.i);
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 228)
    // "..."
    _r0.o = xmlvm_create_java_string_from_pool(110);
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    label73:;
    XMLVM_SOURCE_POSITION("Properties.java", 232)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);
    XMLVM_CHECK_NPE(7)
    java_io_PrintStream_println___java_lang_String(_r7.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 233)
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_setLength___int(_r2.o, _r5.i);
    goto label20;
    label84:;
    XMLVM_SOURCE_POSITION("Properties.java", 223)
    //java_util_Properties_get___java_lang_Object[7]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r4.o)->tib->vtable[7])(_r4.o, _r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 224)
    XMLVM_CHECK_NPE(4)
    _r4.o = ((java_util_Properties*) _r4.o)->fields.java_util_Properties.defaults_;
    goto label49;
    label93:;
    XMLVM_SOURCE_POSITION("Properties.java", 230)
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r1.o);
    goto label73;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_list___java_io_PrintWriter(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_list___java_io_PrintWriter]
    XMLVM_ENTER_METHOD("java.util.Properties", "list", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r6.o = me;
    _r7.o = n1;
    _r5.i = 0;
    XMLVM_SOURCE_POSITION("Properties.java", 247)
    if (_r7.o != JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Properties.java", 248)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label9:;
    XMLVM_SOURCE_POSITION("Properties.java", 250)
    _r2.o = __NEW_java_lang_StringBuilder();
    _r0.i = 80;
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("Properties.java", 251)
    XMLVM_CHECK_NPE(6)
    _r3.o = java_util_Properties_propertyNames__(_r6.o);
    label20:;
    XMLVM_SOURCE_POSITION("Properties.java", 252)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r3.o);
    if (_r0.i != 0) goto label27;
    XMLVM_SOURCE_POSITION("Properties.java", 271)
    XMLVM_EXIT_METHOD()
    return;
    label27:;
    XMLVM_SOURCE_POSITION("Properties.java", 253)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r3.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 254)
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 255)
    _r1.i = 61;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[6])(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 256)
    XMLVM_CHECK_NPE(6)
    _r1.o = java_util_Hashtable_get___java_lang_Object(_r6.o, _r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 257)
    XMLVM_CHECK_NPE(6)
    _r4.o = ((java_util_Properties*) _r6.o)->fields.java_util_Properties.defaults_;
    label49:;
    XMLVM_SOURCE_POSITION("Properties.java", 258)
    if (_r1.o == JAVA_NULL) goto label84;
    XMLVM_SOURCE_POSITION("Properties.java", 262)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r1.o)->tib->vtable[8])(_r1.o);
    _r4.i = 40;
    if (_r0.i <= _r4.i) goto label93;
    XMLVM_SOURCE_POSITION("Properties.java", 263)
    _r0.i = 37;
    XMLVM_CHECK_NPE(1)
    _r0.o = java_lang_String_substring___int_int(_r1.o, _r5.i, _r0.i);
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 264)
    // "..."
    _r0.o = xmlvm_create_java_string_from_pool(110);
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    label73:;
    XMLVM_SOURCE_POSITION("Properties.java", 268)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);

    
    // Red class access removed: java.io.PrintWriter::println
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 269)
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_setLength___int(_r2.o, _r5.i);
    goto label20;
    label84:;
    XMLVM_SOURCE_POSITION("Properties.java", 259)
    //java_util_Properties_get___java_lang_Object[7]
    XMLVM_CHECK_NPE(4)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r4.o)->tib->vtable[7])(_r4.o, _r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 260)
    XMLVM_CHECK_NPE(4)
    _r4.o = ((java_util_Properties*) _r4.o)->fields.java_util_Properties.defaults_;
    goto label49;
    label93:;
    XMLVM_SOURCE_POSITION("Properties.java", 266)
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r1.o);
    goto label73;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_load___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_load___java_io_InputStream]
    XMLVM_ENTER_METHOD("java.util.Properties", "load", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 283)
    java_lang_Object_acquireLockRecursive__(_r3.o);
    if (_r4.o != JAVA_NULL) goto label12;
    XMLVM_TRY_BEGIN(w1975aaac20b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 284)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac20b1b6)
        XMLVM_CATCH_SPECIFIC(w1975aaac20b1b6,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac20b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac20b1b6)
    label9:;
    java_lang_Thread* curThread_w1975aaac20b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac20b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_TRY_BEGIN(w1975aaac20b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 286)

    
    // Red class access removed: java.io.BufferedInputStream::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.BufferedInputStream::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 287)
    _r1.i = 2147483647;

    
    // Red class access removed: java.io.BufferedInputStream::mark
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 288)
    XMLVM_CHECK_NPE(3)
    _r1.i = java_util_Properties_isEbcdic___java_io_BufferedInputStream(_r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 289)

    
    // Red class access removed: java.io.BufferedInputStream::reset
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 291)
    if (_r1.i != 0) { XMLVM_MEMCPY(curThread_w1975aaac20b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac20b1c12, sizeof(XMLVM_JMP_BUF)); goto label44; };
    XMLVM_SOURCE_POSITION("Properties.java", 292)

    
    // Red class access removed: java.io.InputStreamReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "ISO8859-1"
    _r2.o = xmlvm_create_java_string_from_pool(111);

    
    // Red class access removed: java.io.InputStreamReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_util_Properties_loadImpl___java_io_Reader(_r3.o, _r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac20b1c12)
        XMLVM_CATCH_SPECIFIC(w1975aaac20b1c12,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac20b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac20b1c12)
    label42:;
    XMLVM_SOURCE_POSITION("Properties.java", 296)
    java_lang_Object_releaseLockRecursive__(_r3.o);
    XMLVM_EXIT_METHOD()
    return;
    label44:;
    XMLVM_TRY_BEGIN(w1975aaac20b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 294)

    
    // Red class access removed: java.io.InputStreamReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.InputStreamReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(3)
    java_util_Properties_loadImpl___java_io_Reader(_r3.o, _r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac20b1c18)
        XMLVM_CATCH_SPECIFIC(w1975aaac20b1c18,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac20b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac20b1c18)
    goto label42;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_util_Properties_isEbcdic___java_io_BufferedInputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_isEbcdic___java_io_BufferedInputStream]
    XMLVM_ENTER_METHOD("java.util.Properties", "isEbcdic", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    _r2.i = 0;
    label1:;
    XMLVM_SOURCE_POSITION("Properties.java", 300)

    
    // Red class access removed: java.io.BufferedInputStream::read
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.i = (_r0.i << 24) >> 24;
    _r1.i = -1;
    if (_r0.i != _r1.i) goto label11;
    _r0 = _r2;
    label10:;
    XMLVM_SOURCE_POSITION("Properties.java", 311)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("Properties.java", 301)
    _r1.i = 35;
    if (_r0.i == _r1.i) goto label23;
    _r1.i = 10;
    if (_r0.i == _r1.i) goto label23;
    _r1.i = 61;
    if (_r0.i != _r1.i) goto label25;
    label23:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Properties.java", 302)
    goto label10;
    label25:;
    XMLVM_SOURCE_POSITION("Properties.java", 304)
    _r1.i = 21;
    if (_r0.i != _r1.i) goto label1;
    XMLVM_SOURCE_POSITION("Properties.java", 305)
    _r0.i = 1;
    goto label10;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_load___java_io_Reader(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_load___java_io_Reader]
    XMLVM_ENTER_METHOD("java.util.Properties", "load", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 343)
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w1975aaac22b1b5)
    // Begin try
    XMLVM_CHECK_NPE(1)
    java_util_Properties_loadImpl___java_io_Reader(_r1.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac22b1b5)
        XMLVM_CATCH_SPECIFIC(w1975aaac22b1b5,java_lang_Object,6)
    XMLVM_CATCH_END(w1975aaac22b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac22b1b5)
    XMLVM_SOURCE_POSITION("Properties.java", 344)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return;
    label6:;
    java_lang_Thread* curThread_w1975aaac22b1c10 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac22b1c10->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Properties_loadImpl___java_io_Reader(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_loadImpl___java_io_Reader]
    XMLVM_ENTER_METHOD("java.util.Properties", "loadImpl", "?")
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
    XMLVMElem _r15;
    XMLVMElem _r16;
    XMLVMElem _r17;
    XMLVMElem _r18;
    XMLVMElem _r19;
    _r18.o = me;
    _r19.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 347)
    _r3.i = 0;
    _r4.i = 0;
    _r5.i = 0;
    _r6.i = 40;
    XMLVM_SOURCE_POSITION("Properties.java", 348)
    XMLVM_CLASS_INIT(char)
    _r6.o = XMLVMArray_createSingleDimension(__CLASS_char, _r6.i);
    XMLVM_SOURCE_POSITION("Properties.java", 349)
    _r7.i = 0;
    _r8.i = -1;
    _r9.i = 1;
    XMLVM_SOURCE_POSITION("Properties.java", 351)

    
    // Red class access removed: java.io.BufferedReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0 = _r10;
    _r1 = _r19;

    
    // Red class access removed: java.io.BufferedReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    _r15 = _r9;
    _r9 = _r3;
    _r3 = _r15;
    _r16 = _r7;
    _r7 = _r5;
    _r5 = _r16;
    _r17 = _r4;
    _r4 = _r8;
    _r8 = _r17;
    label31:;
    XMLVM_SOURCE_POSITION("Properties.java", 354)

    
    // Red class access removed: java.io.BufferedReader::read
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 355)
    _r12.i = -1;
    if (_r11.i != _r12.i) goto label56;
    XMLVM_SOURCE_POSITION("Properties.java", 488)
    _r3.i = 2;
    if (_r9.i != _r3.i) goto label297;
    _r3.i = 4;
    if (_r7.i > _r3.i) goto label297;
    XMLVM_SOURCE_POSITION("Properties.java", 490)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.08"
    _r4.o = xmlvm_create_java_string_from_pool(112);
    _r4.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r4.o);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label56:;
    XMLVM_SOURCE_POSITION("Properties.java", 356)
    _r11.i = _r11.i & 0xffff;
    XMLVM_SOURCE_POSITION("Properties.java", 358)
    _r12.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    if (_r5.i != _r12.i) goto label71;
    XMLVM_SOURCE_POSITION("Properties.java", 359)
    _r12.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r6.o));
    _r12.i = _r12.i * 2;
    XMLVM_CLASS_INIT(char)
    _r12.o = XMLVMArray_createSingleDimension(__CLASS_char, _r12.i);
    XMLVM_SOURCE_POSITION("Properties.java", 360)
    _r13.i = 0;
    _r14.i = 0;
    java_lang_System_arraycopy___java_lang_Object_int_java_lang_Object_int_int(_r6.o, _r13.i, _r12.o, _r14.i, _r5.i);
    _r6 = _r12;
    label71:;
    XMLVM_SOURCE_POSITION("Properties.java", 361)
    XMLVM_SOURCE_POSITION("Properties.java", 363)
    _r12.i = 2;
    if (_r9.i != _r12.i) goto label122;
    XMLVM_SOURCE_POSITION("Properties.java", 364)
    _r12.i = 16;
    _r12.i = java_lang_Character_digit___char_int(_r11.i, _r12.i);
    XMLVM_SOURCE_POSITION("Properties.java", 365)
    if (_r12.i < 0) goto label106;
    XMLVM_SOURCE_POSITION("Properties.java", 366)
    _r8.i = _r8.i << 4;
    _r8.i = _r8.i + _r12.i;
    _r7.i = _r7.i + 1;
    _r12.i = 4;
    if (_r7.i < _r12.i) goto label31;
    label90:;
    XMLVM_SOURCE_POSITION("Properties.java", 367)
    XMLVM_SOURCE_POSITION("Properties.java", 374)
    _r9.i = 0;
    _r12.i = _r5.i + 1;
    _r13.i = _r8.i & 0xffff;
    XMLVM_SOURCE_POSITION("Properties.java", 375)
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r13.i;
    XMLVM_SOURCE_POSITION("Properties.java", 376)
    _r5.i = 10;
    if (_r11.i == _r5.i) goto label121;
    _r5.i = 133;
    if (_r11.i == _r5.i) goto label121;
    _r5 = _r12;
    XMLVM_SOURCE_POSITION("Properties.java", 377)
    goto label31;
    label106:;
    XMLVM_SOURCE_POSITION("Properties.java", 370)
    _r9.i = 4;
    if (_r7.i > _r9.i) goto label90;
    XMLVM_SOURCE_POSITION("Properties.java", 372)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "luni.09"
    _r4.o = xmlvm_create_java_string_from_pool(113);
    _r4.o = org_apache_harmony_luni_internal_nls_Messages_getString___java_lang_String(_r4.o);

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r3.o)
    label121:;
    _r5 = _r12;
    label122:;
    XMLVM_SOURCE_POSITION("Properties.java", 380)
    _r12.i = 1;
    if (_r9.i != _r12.i) goto label169;
    XMLVM_SOURCE_POSITION("Properties.java", 381)
    _r9.i = 0;
    XMLVM_SOURCE_POSITION("Properties.java", 382)
    switch (_r11.i) {
    case 10: goto label146;
    case 13: goto label144;
    case 98: goto label148;
    case 102: goto label151;
    case 110: goto label154;
    case 114: goto label157;
    case 116: goto label160;
    case 117: goto label163;
    case 133: goto label146;
    }
    _r3 = _r11;
    label130:;
    XMLVM_SOURCE_POSITION("Properties.java", 409)
    XMLVM_SOURCE_POSITION("Properties.java", 481)
    _r11.i = 0;
    _r12.i = 4;
    if (_r9.i != _r12.i) goto label137;
    XMLVM_SOURCE_POSITION("Properties.java", 482)
    XMLVM_SOURCE_POSITION("Properties.java", 484)
    _r4.i = 0;
    _r9 = _r4;
    _r4 = _r5;
    label137:;
    XMLVM_SOURCE_POSITION("Properties.java", 486)
    _r12.i = _r5.i + 1;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r3.i;
    _r3 = _r11;
    _r5 = _r12;
    XMLVM_SOURCE_POSITION("Properties.java", 353)
    goto label31;
    label144:;
    XMLVM_SOURCE_POSITION("Properties.java", 384)
    _r9.i = 3;
    XMLVM_SOURCE_POSITION("Properties.java", 385)
    goto label31;
    label146:;
    XMLVM_SOURCE_POSITION("Properties.java", 388)
    _r9.i = 5;
    XMLVM_SOURCE_POSITION("Properties.java", 389)
    goto label31;
    label148:;
    XMLVM_SOURCE_POSITION("Properties.java", 391)
    _r3.i = 8;
    XMLVM_SOURCE_POSITION("Properties.java", 392)
    goto label130;
    label151:;
    XMLVM_SOURCE_POSITION("Properties.java", 394)
    _r3.i = 12;
    XMLVM_SOURCE_POSITION("Properties.java", 395)
    goto label130;
    label154:;
    XMLVM_SOURCE_POSITION("Properties.java", 397)
    _r3.i = 10;
    XMLVM_SOURCE_POSITION("Properties.java", 398)
    goto label130;
    label157:;
    XMLVM_SOURCE_POSITION("Properties.java", 400)
    _r3.i = 13;
    XMLVM_SOURCE_POSITION("Properties.java", 401)
    goto label130;
    label160:;
    XMLVM_SOURCE_POSITION("Properties.java", 403)
    _r3.i = 9;
    XMLVM_SOURCE_POSITION("Properties.java", 404)
    goto label130;
    label163:;
    XMLVM_SOURCE_POSITION("Properties.java", 406)
    _r7.i = 2;
    _r8.i = 0;
    _r9 = _r7;
    _r7 = _r8;
    XMLVM_SOURCE_POSITION("Properties.java", 408)
    goto label31;
    label169:;
    XMLVM_SOURCE_POSITION("Properties.java", 411)
    switch (_r11.i) {
    case 10: goto label223;
    case 13: goto label229;
    case 33: goto label199;
    case 35: goto label199;
    case 58: goto label277;
    case 61: goto label277;
    case 92: goto label270;
    case 133: goto label229;
    }
    label172:;
    XMLVM_SOURCE_POSITION("Properties.java", 464)
    _r12.i = 256;
    if (_r11.i >= _r12.i) goto label285;
    _r12.i = java_lang_Character_isWhitespace___char(_r11.i);
    if (_r12.i == 0) goto label285;
    XMLVM_SOURCE_POSITION("Properties.java", 465)
    _r12.i = 3;
    if (_r9.i != _r12.i) goto label186;
    XMLVM_SOURCE_POSITION("Properties.java", 466)
    _r9.i = 5;
    label186:;
    XMLVM_SOURCE_POSITION("Properties.java", 469)
    if (_r5.i == 0) goto label31;
    if (_r5.i == _r4.i) goto label31;
    _r12.i = 5;
    if (_r9.i == _r12.i) goto label31;
    XMLVM_SOURCE_POSITION("Properties.java", 472)
    _r12.i = -1;
    if (_r4.i != _r12.i) goto label285;
    XMLVM_SOURCE_POSITION("Properties.java", 473)
    _r9.i = 4;
    XMLVM_SOURCE_POSITION("Properties.java", 474)
    goto label31;
    label199:;
    XMLVM_SOURCE_POSITION("Properties.java", 414)
    if (_r3.i == 0) goto label172;
    label201:;
    XMLVM_SOURCE_POSITION("Properties.java", 416)

    
    // Red class access removed: java.io.BufferedReader::read
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 417)
    _r12.i = -1;
    if (_r11.i == _r12.i) goto label31;
    XMLVM_SOURCE_POSITION("Properties.java", 418)
    _r11.i = _r11.i & 0xffff;
    _r12.i = 13;
    if (_r11.i == _r12.i) goto label31;
    XMLVM_SOURCE_POSITION("Properties.java", 421)
    _r12.i = 10;
    if (_r11.i == _r12.i) goto label31;
    _r12.i = 133;
    if (_r11.i != _r12.i) goto label201;
    goto label31;
    label223:;
    XMLVM_SOURCE_POSITION("Properties.java", 429)
    _r11.i = 3;
    if (_r9.i != _r11.i) goto label229;
    XMLVM_SOURCE_POSITION("Properties.java", 430)
    _r9.i = 5;
    XMLVM_SOURCE_POSITION("Properties.java", 431)
    goto label31;
    label229:;
    XMLVM_SOURCE_POSITION("Properties.java", 436)
    _r3.i = 0;
    _r9.i = 1;
    if (_r5.i > 0) goto label237;
    XMLVM_SOURCE_POSITION("Properties.java", 438)
    if (_r5.i != 0) goto label263;
    if (_r4.i != 0) goto label263;
    label237:;
    XMLVM_SOURCE_POSITION("Properties.java", 439)
    _r11.i = -1;
    if (_r4.i != _r11.i) goto label241;
    _r4 = _r5;
    label241:;
    XMLVM_SOURCE_POSITION("Properties.java", 440)
    XMLVM_SOURCE_POSITION("Properties.java", 442)
    _r11.o = __NEW_java_lang_String();
    _r12.i = 0;
    XMLVM_CHECK_NPE(11)
    java_lang_String___INIT____char_1ARRAY_int_int(_r11.o, _r6.o, _r12.i, _r5.i);
    XMLVM_SOURCE_POSITION("Properties.java", 443)
    _r5.i = 0;
    XMLVM_CHECK_NPE(11)
    _r5.o = java_lang_String_substring___int_int(_r11.o, _r5.i, _r4.i);
    XMLVM_SOURCE_POSITION("Properties.java", 444)
    XMLVM_CHECK_NPE(11)
    _r4.o = java_lang_String_substring___int(_r11.o, _r4.i);
    _r0 = _r18;
    _r1 = _r5;
    _r2 = _r4;
    //java_util_Properties_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r0.o)->tib->vtable[10])(_r0.o, _r1.o, _r2.o);
    label263:;
    XMLVM_SOURCE_POSITION("Properties.java", 446)
    _r4.i = -1;
    _r5.i = 0;
    _r15 = _r9;
    _r9 = _r3;
    _r3 = _r15;
    XMLVM_SOURCE_POSITION("Properties.java", 448)
    goto label31;
    label270:;
    XMLVM_SOURCE_POSITION("Properties.java", 450)
    _r11.i = 4;
    if (_r9.i != _r11.i) goto label274;
    _r4 = _r5;
    label274:;
    XMLVM_SOURCE_POSITION("Properties.java", 451)
    XMLVM_SOURCE_POSITION("Properties.java", 453)
    _r9.i = 1;
    XMLVM_SOURCE_POSITION("Properties.java", 454)
    goto label31;
    label277:;
    XMLVM_SOURCE_POSITION("Properties.java", 457)
    _r12.i = -1;
    if (_r4.i != _r12.i) goto label172;
    XMLVM_SOURCE_POSITION("Properties.java", 458)
    _r4.i = 0;
    _r9 = _r4;
    _r4 = _r5;
    XMLVM_SOURCE_POSITION("Properties.java", 460)
    goto label31;
    label285:;
    _r3 = _r9;
    XMLVM_SOURCE_POSITION("Properties.java", 477)
    _r9.i = 5;
    if (_r3.i == _r9.i) goto label292;
    _r9.i = 3;
    if (_r3.i != _r9.i) goto label352;
    label292:;
    XMLVM_SOURCE_POSITION("Properties.java", 478)
    _r3.i = 0;
    _r9 = _r3;
    _r3 = _r11;
    goto label130;
    label297:;
    XMLVM_SOURCE_POSITION("Properties.java", 492)
    _r3.i = -1;
    if (_r4.i != _r3.i) goto label350;
    if (_r5.i <= 0) goto label350;
    _r3 = _r5;
    label303:;
    XMLVM_SOURCE_POSITION("Properties.java", 493)
    XMLVM_SOURCE_POSITION("Properties.java", 495)
    if (_r3.i < 0) goto label349;
    XMLVM_SOURCE_POSITION("Properties.java", 496)
    _r4.o = __NEW_java_lang_String();
    _r7.i = 0;
    XMLVM_CHECK_NPE(4)
    java_lang_String___INIT____char_1ARRAY_int_int(_r4.o, _r6.o, _r7.i, _r5.i);
    XMLVM_SOURCE_POSITION("Properties.java", 497)
    _r5.i = 0;
    XMLVM_CHECK_NPE(4)
    _r5.o = java_lang_String_substring___int_int(_r4.o, _r5.i, _r3.i);
    XMLVM_SOURCE_POSITION("Properties.java", 498)
    XMLVM_CHECK_NPE(4)
    _r3.o = java_lang_String_substring___int(_r4.o, _r3.i);
    XMLVM_SOURCE_POSITION("Properties.java", 499)
    _r4.i = 1;
    if (_r9.i != _r4.i) goto label342;
    XMLVM_SOURCE_POSITION("Properties.java", 500)
    _r4.o = __NEW_java_lang_StringBuilder();
    _r3.o = java_lang_String_valueOf___java_lang_Object(_r3.o);
    XMLVM_CHECK_NPE(4)
    java_lang_StringBuilder___INIT____java_lang_String(_r4.o, _r3.o);
    // "\000"
    _r3.o = xmlvm_create_java_string_from_pool(114);
    XMLVM_CHECK_NPE(4)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r4.o, _r3.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(3)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r3.o)->tib->vtable[5])(_r3.o);
    label342:;
    XMLVM_SOURCE_POSITION("Properties.java", 502)
    _r0 = _r18;
    _r1 = _r5;
    _r2 = _r3;
    //java_util_Properties_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r0.o)->tib->vtable[10])(_r0.o, _r1.o, _r2.o);
    label349:;
    XMLVM_SOURCE_POSITION("Properties.java", 504)
    XMLVM_EXIT_METHOD()
    return;
    label350:;
    _r3 = _r4;
    goto label303;
    label352:;
    _r9 = _r3;
    _r3 = _r11;
    goto label130;
    label356:;
    label394:;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_propertyNames__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_propertyNames__]
    XMLVM_ENTER_METHOD("java.util.Properties", "propertyNames", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Properties.java", 514)
    _r0.o = __NEW_java_util_Hashtable();
    XMLVM_CHECK_NPE(0)
    java_util_Hashtable___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 515)
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    java_util_Properties_selectProperties___java_util_Hashtable_boolean(_r2.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 516)
    //java_util_Hashtable_keys__[9]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r0.o)->tib->vtable[9])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_stringPropertyNames__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_stringPropertyNames__]
    XMLVM_ENTER_METHOD("java.util.Properties", "stringPropertyNames", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Properties.java", 528)
    _r0.o = __NEW_java_util_Hashtable();
    XMLVM_CHECK_NPE(0)
    java_util_Hashtable___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 529)
    _r1.i = 1;
    XMLVM_CHECK_NPE(2)
    java_util_Properties_selectProperties___java_util_Hashtable_boolean(_r2.o, _r0.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 530)
    //java_util_Hashtable_keySet__[17]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Hashtable*) _r0.o)->tib->vtable[17])(_r0.o);
    _r0.o = java_util_Collections_unmodifiableSet___java_util_Set(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_selectProperties___java_util_Hashtable_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_selectProperties___java_util_Hashtable_boolean]
    XMLVM_ENTER_METHOD("java.util.Properties", "selectProperties", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = me;
    _r5.o = n1;
    _r6.i = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 538)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_Properties*) _r4.o)->fields.java_util_Properties.defaults_;
    if (_r0.o == JAVA_NULL) goto label9;
    XMLVM_SOURCE_POSITION("Properties.java", 539)
    XMLVM_CHECK_NPE(4)
    _r0.o = ((java_util_Properties*) _r4.o)->fields.java_util_Properties.defaults_;
    XMLVM_CHECK_NPE(0)
    java_util_Properties_selectProperties___java_util_Hashtable_boolean(_r0.o, _r5.o, _r6.i);
    label9:;
    XMLVM_SOURCE_POSITION("Properties.java", 542)
    //java_util_Properties_keys__[9]
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Properties*) _r4.o)->tib->vtable[9])(_r4.o);
    label13:;
    XMLVM_SOURCE_POSITION("Properties.java", 544)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r0.o);
    if (_r1.i != 0) goto label20;
    XMLVM_SOURCE_POSITION("Properties.java", 559)
    XMLVM_EXIT_METHOD()
    return;
    label20:;
    XMLVM_SOURCE_POSITION("Properties.java", 545)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 546)
    if (_r6.i == 0) goto label42;
    XMLVM_SOURCE_POSITION("Properties.java", 548)
    XMLVM_CLASS_INIT(java_lang_String)
    _r2.i = XMLVM_ISA(_r1.o, __CLASS_java_lang_String);
    if (_r2.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("Properties.java", 549)
    //java_util_Properties_get___java_lang_Object[7]
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r4.o)->tib->vtable[7])(_r4.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 550)
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.i = XMLVM_ISA(_r2.o, __CLASS_java_lang_String);
    if (_r3.i == 0) goto label13;
    XMLVM_SOURCE_POSITION("Properties.java", 551)
    //java_util_Hashtable_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[10])(_r5.o, _r1.o, _r2.o);
    goto label13;
    label42:;
    XMLVM_SOURCE_POSITION("Properties.java", 555)
    //java_util_Properties_get___java_lang_Object[7]
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r4.o)->tib->vtable[7])(_r4.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 556)
    //java_util_Hashtable_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Hashtable*) _r5.o)->tib->vtable[10])(_r5.o, _r1.o, _r2.o);
    goto label13;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_save___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_save___java_io_OutputStream_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "save", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_TRY_BEGIN(w1975aaac27b1b4)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 578)
    XMLVM_CHECK_NPE(1)
    java_util_Properties_store___java_io_OutputStream_java_lang_String(_r1.o, _r2.o, _r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac27b1b4)
        XMLVM_CATCH_SPECIFIC(w1975aaac27b1b4,java_io_IOException,4)
    XMLVM_CATCH_END(w1975aaac27b1b4)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac27b1b4)
    label3:;
    XMLVM_SOURCE_POSITION("Properties.java", 581)
    XMLVM_EXIT_METHOD()
    return;
    label4:;
    java_lang_Thread* curThread_w1975aaac27b1b9 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac27b1b9->fields.java_lang_Thread.xmlvmException_;
    goto label3;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_setProperty___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_setProperty___java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "setProperty", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 594)
    //java_util_Properties_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r1.o)->tib->vtable[10])(_r1.o, _r2.o, _r3.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_store___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_store___java_io_OutputStream_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "store", "?")
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
    _r7.o = me;
    _r8.o = n1;
    _r9.o = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 615)
    java_lang_Object_acquireLockRecursive__(_r7.o);
    XMLVM_TRY_BEGIN(w1975aaac29b1b6)
    // Begin try
    _r0.o = java_util_Properties_GET_lineSeparator();
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac29b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac29b1b6, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Properties.java", 617)
    _r0.o = __NEW_org_apache_harmony_luni_util_PriviAction();
    // "line.separator"
    _r1.o = xmlvm_create_java_string_from_pool(115);
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String(_r0.o, _r1.o);
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 616)
    java_util_Properties_PUT_lineSeparator( _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac29b1b6)
        XMLVM_CATCH_SPECIFIC(w1975aaac29b1b6,java_lang_Object,130)
    XMLVM_CATCH_END(w1975aaac29b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac29b1b6)
    label20:;
    XMLVM_TRY_BEGIN(w1975aaac29b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 620)
    _r2.o = __NEW_java_lang_StringBuilder();
    _r0.i = 200;
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("Properties.java", 621)

    
    // Red class access removed: java.io.OutputStreamWriter::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    // "ISO8859_1"
    _r0.o = xmlvm_create_java_string_from_pool(116);

    
    // Red class access removed: java.io.OutputStreamWriter::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 622)
    if (_r9.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac29b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac29b1b8, sizeof(XMLVM_JMP_BUF)); goto label39; };
    XMLVM_SOURCE_POSITION("Properties.java", 623)
    XMLVM_CHECK_NPE(7)
    java_util_Properties_writeComments___java_io_Writer_java_lang_String(_r7.o, _r3.o, _r9.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac29b1b8)
        XMLVM_CATCH_SPECIFIC(w1975aaac29b1b8,java_lang_Object,130)
    XMLVM_CATCH_END(w1975aaac29b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac29b1b8)
    label39:;
    XMLVM_TRY_BEGIN(w1975aaac29b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 625)
    _r0.i = 35;

    
    // Red class access removed: java.io.OutputStreamWriter::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 626)

    
    // Red class access removed: java.util.Date::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Date::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Date::toString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.OutputStreamWriter::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 627)
    _r0.o = java_util_Properties_GET_lineSeparator();

    
    // Red class access removed: java.io.OutputStreamWriter::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 629)
    //java_util_Properties_entrySet__[16]
    XMLVM_CHECK_NPE(7)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Properties*) _r7.o)->tib->vtable[16])(_r7.o);
    XMLVM_CHECK_NPE(0)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac29b1c10)
        XMLVM_CATCH_SPECIFIC(w1975aaac29b1c10,java_lang_Object,130)
    XMLVM_CATCH_END(w1975aaac29b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac29b1c10)
    label69:;
    XMLVM_TRY_BEGIN(w1975aaac29b1c12)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r4.o);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1975aaac29b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac29b1c12, sizeof(XMLVM_JMP_BUF)); goto label80; };
    XMLVM_SOURCE_POSITION("Properties.java", 638)

    
    // Red class access removed: java.io.OutputStreamWriter::flush
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac29b1c12)
        XMLVM_CATCH_SPECIFIC(w1975aaac29b1c12,java_lang_Object,130)
    XMLVM_CATCH_END(w1975aaac29b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac29b1c12)
    XMLVM_SOURCE_POSITION("Properties.java", 639)
    java_lang_Object_releaseLockRecursive__(_r7.o);
    XMLVM_EXIT_METHOD()
    return;
    label80:;
    XMLVM_TRY_BEGIN(w1975aaac29b1c17)
    // Begin try
    XMLVM_CHECK_NPE(4)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r4.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 630)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 631)
    _r5.i = 1;
    _r6.i = 1;
    XMLVM_CHECK_NPE(7)
    java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(_r7.o, _r2.o, _r1.o, _r5.i, _r6.i);
    XMLVM_SOURCE_POSITION("Properties.java", 632)
    _r1.i = 61;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[6])(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 633)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    _r0.o = _r0.o;
    _r1.i = 0;
    _r5.i = 1;
    XMLVM_CHECK_NPE(7)
    java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(_r7.o, _r2.o, _r0.o, _r1.i, _r5.i);
    XMLVM_SOURCE_POSITION("Properties.java", 634)
    _r0.o = java_util_Properties_GET_lineSeparator();
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 635)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);

    
    // Red class access removed: java.io.OutputStreamWriter::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 636)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_setLength___int(_r2.o, _r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac29b1c17)
        XMLVM_CATCH_SPECIFIC(w1975aaac29b1c17,java_lang_Object,130)
    XMLVM_CATCH_END(w1975aaac29b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac29b1c17)
    goto label69;
    label130:;
    java_lang_Thread* curThread_w1975aaac29b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac29b1c20->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r7.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Properties_store___java_io_Writer_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_store___java_io_Writer_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "store", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r6.o = me;
    _r7.o = n1;
    _r8.o = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 656)
    java_lang_Object_acquireLockRecursive__(_r6.o);
    XMLVM_TRY_BEGIN(w1975aaac30b1b6)
    // Begin try
    _r0.o = java_util_Properties_GET_lineSeparator();
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac30b1b6->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac30b1b6, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Properties.java", 658)
    _r0.o = __NEW_org_apache_harmony_luni_util_PriviAction();
    // "line.separator"
    _r1.o = xmlvm_create_java_string_from_pool(115);
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_util_PriviAction___INIT____java_lang_String(_r0.o, _r1.o);
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 657)
    java_util_Properties_PUT_lineSeparator( _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac30b1b6)
        XMLVM_CATCH_SPECIFIC(w1975aaac30b1b6,java_lang_Object,123)
    XMLVM_CATCH_END(w1975aaac30b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac30b1b6)
    label20:;
    XMLVM_TRY_BEGIN(w1975aaac30b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 660)
    if (_r8.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac30b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac30b1b8, sizeof(XMLVM_JMP_BUF)); goto label25; };
    XMLVM_SOURCE_POSITION("Properties.java", 661)
    XMLVM_CHECK_NPE(6)
    java_util_Properties_writeComments___java_io_Writer_java_lang_String(_r6.o, _r7.o, _r8.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac30b1b8)
        XMLVM_CATCH_SPECIFIC(w1975aaac30b1b8,java_lang_Object,123)
    XMLVM_CATCH_END(w1975aaac30b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac30b1b8)
    label25:;
    XMLVM_TRY_BEGIN(w1975aaac30b1c10)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 663)
    _r0.i = 35;

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 664)

    
    // Red class access removed: java.util.Date::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Date::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Date::toString
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 665)
    _r0.o = java_util_Properties_GET_lineSeparator();

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 667)
    _r2.o = __NEW_java_lang_StringBuilder();
    _r0.i = 200;
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder___INIT____int(_r2.o, _r0.i);
    XMLVM_SOURCE_POSITION("Properties.java", 668)
    //java_util_Properties_entrySet__[16]
    XMLVM_CHECK_NPE(6)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Properties*) _r6.o)->tib->vtable[16])(_r6.o);
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac30b1c10)
        XMLVM_CATCH_SPECIFIC(w1975aaac30b1c10,java_lang_Object,123)
    XMLVM_CATCH_END(w1975aaac30b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac30b1c10)
    label62:;
    XMLVM_TRY_BEGIN(w1975aaac30b1c12)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r3.o);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1975aaac30b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac30b1c12, sizeof(XMLVM_JMP_BUF)); goto label73; };
    XMLVM_SOURCE_POSITION("Properties.java", 677)

    
    // Red class access removed: java.io.Writer::flush
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac30b1c12)
        XMLVM_CATCH_SPECIFIC(w1975aaac30b1c12,java_lang_Object,123)
    XMLVM_CATCH_END(w1975aaac30b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac30b1c12)
    XMLVM_SOURCE_POSITION("Properties.java", 678)
    java_lang_Object_releaseLockRecursive__(_r6.o);
    XMLVM_EXIT_METHOD()
    return;
    label73:;
    XMLVM_TRY_BEGIN(w1975aaac30b1c17)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r3.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 669)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 670)
    _r4.i = 1;
    _r5.i = 0;
    XMLVM_CHECK_NPE(6)
    java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(_r6.o, _r2.o, _r1.o, _r4.i, _r5.i);
    XMLVM_SOURCE_POSITION("Properties.java", 671)
    _r1.i = 61;
    //java_lang_StringBuilder_append___char[6]
    XMLVM_CHECK_NPE(2)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_CHAR)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[6])(_r2.o, _r1.i);
    XMLVM_SOURCE_POSITION("Properties.java", 672)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    _r0.o = _r0.o;
    _r1.i = 0;
    _r4.i = 0;
    XMLVM_CHECK_NPE(6)
    java_util_Properties_dumpString___java_lang_StringBuilder_java_lang_String_boolean_boolean(_r6.o, _r2.o, _r0.o, _r1.i, _r4.i);
    XMLVM_SOURCE_POSITION("Properties.java", 673)
    _r0.o = java_util_Properties_GET_lineSeparator();
    XMLVM_CHECK_NPE(2)
    java_lang_StringBuilder_append___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 674)
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r2.o)->tib->vtable[5])(_r2.o);

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 675)
    _r0.i = 0;
    XMLVM_CHECK_NPE(2)
    java_lang_AbstractStringBuilder_setLength___int(_r2.o, _r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac30b1c17)
        XMLVM_CATCH_SPECIFIC(w1975aaac30b1c17,java_lang_Object,123)
    XMLVM_CATCH_END(w1975aaac30b1c17)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac30b1c17)
    goto label62;
    label123:;
    java_lang_Thread* curThread_w1975aaac30b1c20 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac30b1c20->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r6.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    //XMLVM_END_WRAPPER
}

void java_util_Properties_writeComments___java_io_Writer_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_writeComments___java_io_Writer_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "writeComments", "?")
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
    _r7.o = me;
    _r8.o = n1;
    _r9.o = n2;
    _r6.i = 13;
    _r5.i = 10;
    _r4.i = 35;
    XMLVM_SOURCE_POSITION("Properties.java", 682)

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 683)
    XMLVM_CHECK_NPE(9)
    _r0.o = java_lang_String_toCharArray__(_r9.o);
    XMLVM_SOURCE_POSITION("Properties.java", 684)
    _r1.i = 0;
    label14:;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r1.i < _r2.i) goto label23;
    XMLVM_SOURCE_POSITION("Properties.java", 707)
    _r0.o = java_util_Properties_GET_lineSeparator();

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 708)
    XMLVM_EXIT_METHOD()
    return;
    label23:;
    XMLVM_SOURCE_POSITION("Properties.java", 685)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = 256;
    if (_r2.i >= _r3.i) goto label81;
    XMLVM_SOURCE_POSITION("Properties.java", 686)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.i == _r6.i) goto label37;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r2.i != _r5.i) goto label75;
    label37:;
    XMLVM_SOURCE_POSITION("Properties.java", 687)
    _r2.i = _r1.i + 1;
    XMLVM_SOURCE_POSITION("Properties.java", 688)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    if (_r3.i != _r6.i) goto label53;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r2.i >= _r3.i) goto label53;
    XMLVM_SOURCE_POSITION("Properties.java", 689)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r3.i != _r5.i) goto label53;
    label50:;
    _r1.i = _r1.i + 1;
    goto label14;
    label53:;
    XMLVM_SOURCE_POSITION("Properties.java", 693)
    _r3.o = java_util_Properties_GET_lineSeparator();

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 694)
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r0.o));
    if (_r2.i >= _r3.i) goto label71;
    XMLVM_SOURCE_POSITION("Properties.java", 695)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r3.i == _r4.i) goto label50;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r3.i = 33;
    if (_r2.i == _r3.i) goto label50;
    label71:;
    XMLVM_SOURCE_POSITION("Properties.java", 699)

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label50;
    label75:;
    XMLVM_SOURCE_POSITION("Properties.java", 701)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label50;
    label81:;
    XMLVM_SOURCE_POSITION("Properties.java", 704)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_CHECK_NPE(7)
    _r2.o = java_util_Properties_toHexaDecimal___int(_r7.o, _r2.i);

    
    // Red class access removed: java.io.Writer::write
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label50;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_loadFromXML___java_io_InputStream(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_loadFromXML___java_io_InputStream]
    XMLVM_ENTER_METHOD("java.util.Properties", "loadFromXML", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    _r5.o = me;
    _r6.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 730)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    if (_r6.o != JAVA_NULL) goto label12;
    XMLVM_TRY_BEGIN(w1975aaac32b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 731)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1b6)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1b6,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1b6)
    label9:;
    java_lang_Thread* curThread_w1975aaac32b1b8 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac32b1b8->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_TRY_BEGIN(w1975aaac32b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 734)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_Properties*) _r5.o)->fields.java_util_Properties.builder_;
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac32b1c12->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac32b1c12, sizeof(XMLVM_JMP_BUF)); goto label50; };
    XMLVM_SOURCE_POSITION("Properties.java", 736)

    
    // Red class access removed: javax.xml.parsers.DocumentBuilderFactory::newInstance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 737)
    _r1.i = 1;

    
    // Red class access removed: javax.xml.parsers.DocumentBuilderFactory::setValidating
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c12)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c12,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c12)
    XMLVM_TRY_BEGIN(w1975aaac32b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 740)

    
    // Red class access removed: javax.xml.parsers.DocumentBuilderFactory::newDocumentBuilder
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(5)
    ((java_util_Properties*) _r5.o)->fields.java_util_Properties.builder_ = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c13)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c13,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c13)
    XMLVM_TRY_BEGIN(w1975aaac32b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 745)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_Properties*) _r5.o)->fields.java_util_Properties.builder_;

    
    // Red class access removed: java.util.Properties$1::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Properties$1::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: javax.xml.parsers.DocumentBuilder::setErrorHandler
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 759)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_Properties*) _r5.o)->fields.java_util_Properties.builder_;

    
    // Red class access removed: java.util.Properties$2::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Properties$2::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: javax.xml.parsers.DocumentBuilder::setEntityResolver
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c14)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c14,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c14)
    label50:;
    XMLVM_TRY_BEGIN(w1975aaac32b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 775)
    XMLVM_CHECK_NPE(5)
    _r0.o = ((java_util_Properties*) _r5.o)->fields.java_util_Properties.builder_;

    
    // Red class access removed: javax.xml.parsers.DocumentBuilder::parse
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 776)
    // "entry"
    _r1.o = xmlvm_create_java_string_from_pool(117);

    
    // Red class access removed: org.w3c.dom.Document::getElementsByTagName
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c16)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c16,java_io_IOException,104)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c16,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c16)
    XMLVM_SOURCE_POSITION("Properties.java", 777)
    if (_r1.o != JAVA_NULL) goto label73;
    label64:;
    XMLVM_SOURCE_POSITION("Properties.java", 798)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return;
    label66:;
    XMLVM_SOURCE_POSITION("Properties.java", 741)
    java_lang_Thread* curThread_w1975aaac32b1c25 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac32b1c25->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w1975aaac32b1c26)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 742)

    
    // Red class access removed: java.lang.Error::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.Error::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c26)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c26,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c26)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c26)
    label73:;
    XMLVM_TRY_BEGIN(w1975aaac32b1c28)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 780)

    
    // Red class access removed: org.w3c.dom.NodeList::getLength
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 782)
    _r0.i = 0;
    _r3 = _r0;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c28)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c28,java_io_IOException,104)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c28,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c28)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c28)
    label79:;
    XMLVM_TRY_BEGIN(w1975aaac32b1c30)
    // Begin try
    if (_r3.i >= _r2.i) { XMLVM_MEMCPY(curThread_w1975aaac32b1c30->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac32b1c30, sizeof(XMLVM_JMP_BUF)); goto label64; };
    XMLVM_SOURCE_POSITION("Properties.java", 783)

    
    // Red class access removed: org.w3c.dom.NodeList::item
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: org.w3c.dom.Element::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 784)
    // "key"
    _r4.o = xmlvm_create_java_string_from_pool(118);

    
    // Red class access removed: org.w3c.dom.Element::getAttribute
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 785)

    
    // Red class access removed: org.w3c.dom.Element::getTextContent
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Properties.java", 791)
    //java_util_Properties_put___java_lang_Object_java_lang_Object[10]
    XMLVM_CHECK_NPE(5)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_Properties*) _r5.o)->tib->vtable[10])(_r5.o, _r4.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c30)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c30,java_io_IOException,104)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c30,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c30)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c30)
    _r0.i = _r3.i + 1;
    _r3 = _r0;
    goto label79;
    label104:;
    XMLVM_SOURCE_POSITION("Properties.java", 793)
    java_lang_Thread* curThread_w1975aaac32b1c36 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac32b1c36->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w1975aaac32b1c37)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 794)
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c37)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c37,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c37)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c37)
    label106:;
    XMLVM_TRY_BEGIN(w1975aaac32b1c39)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 795)
    java_lang_Thread* curThread_w1975aaac32b1c39ab1 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac32b1c39ab1->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("Properties.java", 796)

    
    // Red class access removed: java.util.InvalidPropertiesFormatException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.InvalidPropertiesFormatException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac32b1c39)
        XMLVM_CATCH_SPECIFIC(w1975aaac32b1c39,java_lang_Object,9)
    XMLVM_CATCH_END(w1975aaac32b1c39)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac32b1c39)
    //XMLVM_END_WRAPPER
}

void java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "storeToXML", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r1.o = me;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Properties.java", 818)
    // "UTF-8"
    _r0.o = xmlvm_create_java_string_from_pool(100);
    XMLVM_CHECK_NPE(1)
    java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String_java_lang_String(_r1.o, _r2.o, _r3.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 819)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_storeToXML___java_io_OutputStream_java_lang_String_java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "storeToXML", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r5.o = me;
    _r6.o = n1;
    _r7.o = n2;
    _r8.o = n3;
    // "UTF-8"
    _r3.o = xmlvm_create_java_string_from_pool(100);
    // "\042>"
    _r0.o = xmlvm_create_java_string_from_pool(119);
    XMLVM_SOURCE_POSITION("Properties.java", 842)
    java_lang_Object_acquireLockRecursive__(_r5.o);
    if (_r6.o == JAVA_NULL) goto label9;
    if (_r8.o != JAVA_NULL) goto label18;
    label9:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c12)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 843)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c12)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c12,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c12)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c12)
    label15:;
    java_lang_Thread* curThread_w1975aaac34b1c14 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac34b1c14->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label18:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 855)
    _r0.o = java_nio_charset_Charset_forName___java_lang_String(_r8.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_Charset_name__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c18)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c18,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c18)
    label26:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c20)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 866)
    _r2.o = __NEW_java_io_PrintStream();
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream___INIT____java_io_OutputStream_boolean_java_lang_String(_r2.o, _r6.o, _r1.i, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 869)
    // "<?xml version=\0421.0\042 encoding=\042"
    _r1.o = xmlvm_create_java_string_from_pool(120);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 870)
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 871)
    // "\042?>"
    _r0.o = xmlvm_create_java_string_from_pool(121);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 873)
    // "<!DOCTYPE properties SYSTEM \042"
    _r0.o = xmlvm_create_java_string_from_pool(122);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 874)
    // "http://java.sun.com/dtd/properties.dtd"
    _r0.o = xmlvm_create_java_string_from_pool(102);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 875)
    // "\042>"
    _r0.o = xmlvm_create_java_string_from_pool(119);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 877)
    // "<properties>"
    _r0.o = xmlvm_create_java_string_from_pool(123);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 879)
    if (_r7.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w1975aaac34b1c20->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac34b1c20, sizeof(XMLVM_JMP_BUF)); goto label84; };
    XMLVM_SOURCE_POSITION("Properties.java", 880)
    // "<comment>"
    _r0.o = xmlvm_create_java_string_from_pool(124);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 881)
    XMLVM_CHECK_NPE(5)
    _r0.o = java_util_Properties_substitutePredefinedEntries___java_lang_String(_r5.o, _r7.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 882)
    // "</comment>"
    _r0.o = xmlvm_create_java_string_from_pool(125);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c20)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c20,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c20)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c20)
    label84:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Properties.java", 885)
    //java_util_Properties_entrySet__[16]
    XMLVM_CHECK_NPE(5)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_Properties*) _r5.o)->tib->vtable[16])(_r5.o);
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c22)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c22,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c22)
    label92:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c24)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r3.o);
    if (_r0.i != 0) { XMLVM_MEMCPY(curThread_w1975aaac34b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac34b1c24, sizeof(XMLVM_JMP_BUF)); goto label171; };
    XMLVM_SOURCE_POSITION("Properties.java", 894)
    // "</properties>"
    _r0.o = xmlvm_create_java_string_from_pool(126);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 895)
    //java_io_PrintStream_flush__[8]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT)) ((java_io_PrintStream*) _r2.o)->tib->vtable[8])(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c24)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c24,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c24)
    XMLVM_SOURCE_POSITION("Properties.java", 896)
    java_lang_Object_releaseLockRecursive__(_r5.o);
    XMLVM_EXIT_METHOD()
    return;
    label108:;
    XMLVM_SOURCE_POSITION("Properties.java", 857)
    java_lang_Thread* curThread_w1975aaac34b1c30 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac34b1c30->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w1975aaac34b1c31)
    // Begin try
    _r0.o = java_lang_System_GET_out();
    _r1.o = __NEW_java_lang_StringBuilder();
    // "Warning: encoding name "
    _r2.o = xmlvm_create_java_string_from_pool(127);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r8.o);
    XMLVM_SOURCE_POSITION("Properties.java", 858)
    // " is illegal, using UTF-8 as default encoding"
    _r2.o = xmlvm_create_java_string_from_pool(128);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_io_PrintStream_println___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 859)
    // "UTF-8"
    _r0.o = xmlvm_create_java_string_from_pool(100);
    _r0 = _r3;
    { XMLVM_MEMCPY(curThread_w1975aaac34b1c31->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac34b1c31, sizeof(XMLVM_JMP_BUF)); goto label26; };
    XMLVM_SOURCE_POSITION("Properties.java", 861)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c31)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c31,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c31)
    label139:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c33)
    // Begin try
    java_lang_Thread* curThread_w1975aaac34b1c33aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w1975aaac34b1c33aa->fields.java_lang_Thread.xmlvmException_;
    _r0.o = java_lang_System_GET_out();
    _r1.o = __NEW_java_lang_StringBuilder();
    // "Warning: encoding "
    _r2.o = xmlvm_create_java_string_from_pool(129);
    XMLVM_CHECK_NPE(1)
    java_lang_StringBuilder___INIT____java_lang_String(_r1.o, _r2.o);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r8.o);
    XMLVM_SOURCE_POSITION("Properties.java", 862)
    // " is not supported, using UTF-8 as default encoding"
    _r2.o = xmlvm_create_java_string_from_pool(130);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_StringBuilder_append___java_lang_String(_r1.o, _r2.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(1)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r1.o)->tib->vtable[5])(_r1.o);
    XMLVM_CHECK_NPE(0)
    java_io_PrintStream_println___java_lang_String(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 863)
    // "UTF-8"
    _r0.o = xmlvm_create_java_string_from_pool(100);
    _r0 = _r3;
    { XMLVM_MEMCPY(curThread_w1975aaac34b1c33->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w1975aaac34b1c33, sizeof(XMLVM_JMP_BUF)); goto label26; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c33)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c33,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c33)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c33)
    label171:;
    XMLVM_TRY_BEGIN(w1975aaac34b1c35)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r3.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 886)
    XMLVM_CHECK_NPE(0)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(_r0.o);
    _r1.o = _r1.o;
    XMLVM_SOURCE_POSITION("Properties.java", 887)
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Properties.java", 888)
    // "<entry key=\042"
    _r4.o = xmlvm_create_java_string_from_pool(131);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r4.o);
    XMLVM_SOURCE_POSITION("Properties.java", 889)
    XMLVM_CHECK_NPE(5)
    _r1.o = java_util_Properties_substitutePredefinedEntries___java_lang_String(_r5.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 890)
    // "\042>"
    _r1.o = xmlvm_create_java_string_from_pool(119);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r1.o);
    XMLVM_SOURCE_POSITION("Properties.java", 891)
    XMLVM_CHECK_NPE(5)
    _r0.o = java_util_Properties_substitutePredefinedEntries___java_lang_String(_r5.o, _r0.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_print___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Properties.java", 892)
    // "</entry>"
    _r0.o = xmlvm_create_java_string_from_pool(132);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w1975aaac34b1c35)
        XMLVM_CATCH_SPECIFIC(w1975aaac34b1c35,java_lang_Object,15)
    XMLVM_CATCH_END(w1975aaac34b1c35)
    XMLVM_RESTORE_EXCEPTION_ENV(w1975aaac34b1c35)
    goto label92;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_util_Properties_substitutePredefinedEntries___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_util_Properties_substitutePredefinedEntries___java_lang_String]
    XMLVM_ENTER_METHOD("java.util.Properties", "substitutePredefinedEntries", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Properties.java", 904)
    // "&"
    _r0.o = xmlvm_create_java_string_from_pool(133);
    // "&amp;"
    _r1.o = xmlvm_create_java_string_from_pool(134);
    XMLVM_CHECK_NPE(4)
    _r0.o = java_lang_String_replaceAll___java_lang_String_java_lang_String(_r4.o, _r0.o, _r1.o);
    // "<"
    _r1.o = xmlvm_create_java_string_from_pool(135);
    // "&lt;"
    _r2.o = xmlvm_create_java_string_from_pool(136);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replaceAll___java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    XMLVM_SOURCE_POSITION("Properties.java", 905)
    // ">"
    _r1.o = xmlvm_create_java_string_from_pool(137);
    // "&gt;"
    _r2.o = xmlvm_create_java_string_from_pool(138);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replaceAll___java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    // "'"
    _r1.o = xmlvm_create_java_string_from_pool(139);
    // "&apos;"
    _r2.o = xmlvm_create_java_string_from_pool(140);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replaceAll___java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    // "\042"
    _r1.o = xmlvm_create_java_string_from_pool(141);
    XMLVM_SOURCE_POSITION("Properties.java", 906)
    // "&quot;"
    _r2.o = xmlvm_create_java_string_from_pool(142);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_replaceAll___java_lang_String_java_lang_String(_r0.o, _r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

