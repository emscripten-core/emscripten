#include "xmlvm.h"
#include "java_lang_Object.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_nio_charset_Charset.h"
#include "java_security_AccessController.h"
#include "java_util_ArrayList.h"
#include "java_util_Collections.h"
#include "java_util_HashMap.h"
#include "java_util_Iterator.h"
#include "java_util_Map.h"
#include "org_apache_harmony_niochar_CharsetProviderImpl_1.h"

#include "org_apache_harmony_niochar_CharsetProviderImpl.h"

#define XMLVM_CURRENT_CLASS_NAME CharsetProviderImpl
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_niochar_CharsetProviderImpl

__TIB_DEFINITION_org_apache_harmony_niochar_CharsetProviderImpl __TIB_org_apache_harmony_niochar_CharsetProviderImpl = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_niochar_CharsetProviderImpl, // classInitializer
    "org.apache.harmony.niochar.CharsetProviderImpl", // className
    "org.apache.harmony.niochar", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_nio_charset_spi_CharsetProvider, // extends
    sizeof(org_apache_harmony_niochar_CharsetProviderImpl), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_BOOLEAN _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_HAS_LOADED_NATIVES;
static JAVA_INT _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_CLASS;
static JAVA_INT _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_INSTANCE;
static JAVA_INT _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_ALIASES;

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

void __INIT_org_apache_harmony_niochar_CharsetProviderImpl()
{
    staticInitializerLock(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_niochar_CharsetProviderImpl.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_niochar_CharsetProviderImpl.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_niochar_CharsetProviderImpl.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_niochar_CharsetProviderImpl.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.niochar.CharsetProviderImpl")
        __INIT_IMPL_org_apache_harmony_niochar_CharsetProviderImpl();
    }
}

void __INIT_IMPL_org_apache_harmony_niochar_CharsetProviderImpl()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_nio_charset_spi_CharsetProvider)
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_niochar_CharsetProviderImpl;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_niochar_CharsetProviderImpl.vtable, __TIB_java_nio_charset_spi_CharsetProvider.vtable, sizeof(__TIB_java_nio_charset_spi_CharsetProvider.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.vtable[7] = (VTABLE_PTR) &org_apache_harmony_niochar_CharsetProviderImpl_charsets__;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.vtable[6] = (VTABLE_PTR) &org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String;
    // Initialize interface information
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.numImplementedInterfaces = 0;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_HAS_LOADED_NATIVES = 0;
    _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_CLASS = 0;
    _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_INSTANCE = 1;
    _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_ALIASES = 2;

    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_niochar_CharsetProviderImpl);
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.clazz = __CLASS_org_apache_harmony_niochar_CharsetProviderImpl;
    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl_1ARRAY);
    __CLASS_org_apache_harmony_niochar_CharsetProviderImpl_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_niochar_CharsetProviderImpl_2ARRAY);
    org_apache_harmony_niochar_CharsetProviderImpl___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_niochar_CharsetProviderImpl]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_niochar_CharsetProviderImpl.classInitialized = 1;
}

void __DELETE_org_apache_harmony_niochar_CharsetProviderImpl(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_niochar_CharsetProviderImpl]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_nio_charset_spi_CharsetProvider(me, 0 || derivedClassWillRegisterFinalizer);
    ((org_apache_harmony_niochar_CharsetProviderImpl*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl.cache_ = (java_util_Map*) JAVA_NULL;
    ((org_apache_harmony_niochar_CharsetProviderImpl*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    ((org_apache_harmony_niochar_CharsetProviderImpl*) me)->fields.org_apache_harmony_niochar_CharsetProviderImpl.packageName_ = (java_lang_String*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_niochar_CharsetProviderImpl()
{    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
org_apache_harmony_niochar_CharsetProviderImpl* me = (org_apache_harmony_niochar_CharsetProviderImpl*) XMLVM_MALLOC(sizeof(org_apache_harmony_niochar_CharsetProviderImpl));
    me->tib = &__TIB_org_apache_harmony_niochar_CharsetProviderImpl;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_niochar_CharsetProviderImpl(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_niochar_CharsetProviderImpl]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_niochar_CharsetProviderImpl()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_org_apache_harmony_niochar_CharsetProviderImpl();
    org_apache_harmony_niochar_CharsetProviderImpl___INIT___(me);
    return me;
}

JAVA_BOOLEAN org_apache_harmony_niochar_CharsetProviderImpl_GET_HAS_LOADED_NATIVES()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    return _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_HAS_LOADED_NATIVES;
}

void org_apache_harmony_niochar_CharsetProviderImpl_PUT_HAS_LOADED_NATIVES(JAVA_BOOLEAN v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
_STATIC_org_apache_harmony_niochar_CharsetProviderImpl_HAS_LOADED_NATIVES = v;
}

JAVA_INT org_apache_harmony_niochar_CharsetProviderImpl_GET_CHARSET_CLASS()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    return _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_CLASS;
}

void org_apache_harmony_niochar_CharsetProviderImpl_PUT_CHARSET_CLASS(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
_STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_CLASS = v;
}

JAVA_INT org_apache_harmony_niochar_CharsetProviderImpl_GET_CHARSET_INSTANCE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    return _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_INSTANCE;
}

void org_apache_harmony_niochar_CharsetProviderImpl_PUT_CHARSET_INSTANCE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
_STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_INSTANCE = v;
}

JAVA_INT org_apache_harmony_niochar_CharsetProviderImpl_GET_CHARSET_ALIASES()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    return _STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_ALIASES;
}

void org_apache_harmony_niochar_CharsetProviderImpl_PUT_CHARSET_ALIASES(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
_STATIC_org_apache_harmony_niochar_CharsetProviderImpl_CHARSET_ALIASES = v;
}

JAVA_BOOLEAN org_apache_harmony_niochar_CharsetProviderImpl_hasLoadedNatives__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_hasLoadedNatives__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "hasLoadedNatives", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 86)
    _r0.i = org_apache_harmony_niochar_CharsetProviderImpl_GET_HAS_LOADED_NATIVES();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_toUpperCase___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_toUpperCase___java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.o = n1;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 108)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(5)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r5.o)->tib->vtable[8])(_r5.o);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 109)
    XMLVM_CLASS_INIT(char)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    _r1.i = 0;
    label7:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 110)
    if (_r1.i >= _r2.i) goto label46;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 111)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(5)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r5.o)->tib->vtable[6])(_r5.o, _r1.i);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 112)
    _r4.i = org_apache_harmony_niochar_CharsetProviderImpl_passthru___char(_r0.i);
    if (_r4.i == 0) goto label24;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 113)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r0.i;
    label21:;
    _r1.i = _r1.i + 1;
    goto label7;
    label24:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 115)
    _r4.i = 97;
    if (_r0.i < _r4.i) goto label40;
    _r4.i = 122;
    if (_r0.i > _r4.i) goto label40;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 117)
    _r4.i = 32;
    _r4.i = _r0.i - _r4.i;
    _r4.i = _r4.i & 0xffff;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    goto label21;
    label40:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 119)

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r4.o)
    label46:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 123)
    _r4.o = __NEW_java_lang_String();
    XMLVM_CHECK_NPE(4)
    java_lang_String___INIT____char_1ARRAY(_r4.o, _r3.o);
    XMLVM_EXIT_METHOD()
    return _r4.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_apache_harmony_niochar_CharsetProviderImpl_passthru___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(org_apache_harmony_niochar_CharsetProviderImpl)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_passthru___char]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "passthru", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 130)
    _r0.i = 65;
    if (_r1.i < _r0.i) goto label8;
    _r0.i = 90;
    if (_r1.i <= _r0.i) goto label32;
    label8:;
    _r0.i = 48;
    if (_r1.i < _r0.i) goto label16;
    _r0.i = 57;
    if (_r1.i <= _r0.i) goto label32;
    label16:;
    _r0.i = 45;
    if (_r1.i == _r0.i) goto label32;
    _r0.i = 46;
    if (_r1.i == _r0.i) goto label32;
    _r0.i = 58;
    if (_r1.i == _r0.i) goto label32;
    _r0.i = 95;
    if (_r1.i != _r0.i) goto label34;
    label32:;
    _r0.i = 1;
    label33:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label34:;
    _r0.i = 0;
    goto label33;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_CharsetProviderImpl___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl___INIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.o = me;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 141)
    XMLVM_CHECK_NPE(7)
    java_nio_charset_spi_CharsetProvider___INIT___(_r7.o);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 142)
    _r4.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(4)
    java_util_HashMap___INIT___(_r4.o);
    _r4.o = java_util_Collections_synchronizedMap___java_util_Map(_r4.o);
    XMLVM_CHECK_NPE(7)
    ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.cache_ = _r4.o;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 143)
    XMLVM_CHECK_NPE(7)
    _r4.o = org_apache_harmony_niochar_CharsetProviderImpl_getCharsetsInfo__(_r7.o);
    XMLVM_CHECK_NPE(7)
    ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_ = _r4.o;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 144)
    XMLVM_CHECK_NPE(7)
    _r4.o = org_apache_harmony_niochar_CharsetProviderImpl_getPackageName__(_r7.o);
    XMLVM_CHECK_NPE(7)
    ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.packageName_ = _r4.o;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 145)
    _r3.i = 0;
    label27:;
    XMLVM_CHECK_NPE(7)
    _r4.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r3.i >= _r4.i) goto label70;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 146)
    XMLVM_CHECK_NPE(7)
    _r4.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r4.o = ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r5.i = 2;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r4.o = _r4.o;
    _r0 = _r4;
    _r0.o = _r0.o;
    _r2 = _r0;
    _r1.i = 0;
    label46:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 147)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r1.i >= _r4.i) goto label67;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 148)
    XMLVM_CHECK_NPE(7)
    _r4.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.cache_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r5.o = org_apache_harmony_niochar_CharsetProviderImpl_toUpperCase___java_lang_String(_r5.o);
    XMLVM_CHECK_NPE(7)
    _r6.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r7.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r3.i);
    _r6.o = ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(4)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object])(_r4.o, _r5.o, _r6.o);
    _r1.i = _r1.i + 1;
    goto label46;
    label67:;
    _r3.i = _r3.i + 1;
    goto label27;
    label70:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 151)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_charsets__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_charsets__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "charsets", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = me;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 160)
    _r1.o = __NEW_java_util_ArrayList();
    XMLVM_CHECK_NPE(1)
    java_util_ArrayList___INIT___(_r1.o);
    _r0.i = 0;
    label6:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 161)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r4.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r0.i >= _r2.i) goto label35;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 162)
    XMLVM_CHECK_NPE(4)
    _r2.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r4.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.charsets_;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r2.o = ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r3.i = 2;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r2.o = _r2.o;
    _r2.o = _r2.o;
    _r3.i = 0;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    //org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String[6]
    XMLVM_CHECK_NPE(4)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((org_apache_harmony_niochar_CharsetProviderImpl*) _r4.o)->tib->vtable[6])(_r4.o, _r2.o);
    //java_util_ArrayList_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[7])(_r1.o, _r2.o);
    _r0.i = _r0.i + 1;
    goto label6;
    label35:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 165)
    //java_util_ArrayList_iterator__[12]
    XMLVM_CHECK_NPE(1)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) _r1.o)->tib->vtable[12])(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r2.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "charsetForName", "?")
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
    _r10.o = me;
    _r11.o = n1;
    _r9.i = 2;
    _r8.i = 0;
    _r7.i = 1;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 180)
    XMLVM_CHECK_NPE(10)
    _r5.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r10.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.cache_;
    _r6.o = org_apache_harmony_niochar_CharsetProviderImpl_toUpperCase___java_lang_String(_r11.o);
    XMLVM_CHECK_NPE(5)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r5.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_get___java_lang_Object])(_r5.o, _r6.o);
    _r2.o = _r2.o;
    if (_r2.o != JAVA_NULL) goto label19;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 181)
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 182)
    _r5.o = JAVA_NULL;
    label18:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 207)
    XMLVM_EXIT_METHOD()
    return _r5.o;
    label19:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 185)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    if (_r5.o != JAVA_NULL) goto label79;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 186)
    _r5.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(5)
    java_lang_StringBuilder___INIT___(_r5.o);
    XMLVM_CHECK_NPE(10)
    _r6.o = ((org_apache_harmony_niochar_CharsetProviderImpl*) _r10.o)->fields.org_apache_harmony_niochar_CharsetProviderImpl.packageName_;
    XMLVM_CHECK_NPE(5)
    _r5.o = java_lang_StringBuilder_append___java_lang_String(_r5.o, _r6.o);
    // "."
    _r6.o = xmlvm_create_java_string_from_pool(52);
    XMLVM_CHECK_NPE(5)
    _r6.o = java_lang_StringBuilder_append___java_lang_String(_r5.o, _r6.o);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i];
    _r5.o = _r5.o;
    XMLVM_CHECK_NPE(6)
    _r5.o = java_lang_StringBuilder_append___java_lang_String(_r6.o, _r5.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(5)
    _r4.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r5.o)->tib->vtable[5])(_r5.o);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 188)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r9.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i];
    _r5.o = _r5.o;
    _r5.o = _r5.o;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r8.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i];
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 189)
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r9.i);
    _r5.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i];
    _r5.o = _r5.o;
    _r0 = _r5;
    _r0.o = _r0.o;
    _r1 = _r0;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 190)
    _r5.o = __NEW_org_apache_harmony_niochar_CharsetProviderImpl_1();
    XMLVM_CHECK_NPE(5)
    org_apache_harmony_niochar_CharsetProviderImpl_1___INIT____org_apache_harmony_niochar_CharsetProviderImpl_java_lang_String_java_lang_String_java_lang_String_1ARRAY(_r5.o, _r10.o, _r4.o, _r3.o, _r1.o);
    _r5.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r5.o);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r5.o;
    label79:;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    _r10.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i];
    _r10.o = _r10.o;
    _r5 = _r10;
    goto label18;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_CharsetProviderImpl_putCharsets___java_util_Map(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_putCharsets___java_util_Map]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "putCharsets", "?")
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
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 221)
    XMLVM_CHECK_NPE(6)
    _r1.o = org_apache_harmony_niochar_CharsetProviderImpl_getCharsetsInfo__(_r6.o);
    _r3.i = 0;
    label5:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 222)
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r3.i >= _r4.i) goto label32;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 223)
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r3.i);
    _r4.o = ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r5.i = 2;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r4.o = _r4.o;
    _r4.o = _r4.o;
    _r5.i = 0;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 224)
    //org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String[6]
    XMLVM_CHECK_NPE(6)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((org_apache_harmony_niochar_CharsetProviderImpl*) _r6.o)->tib->vtable[6])(_r6.o, _r0.o);
    if (_r2.o == JAVA_NULL) goto label29;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 225)
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 226)
    XMLVM_CHECK_NPE(7)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) *(((java_lang_Object*)_r7.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_put___java_lang_Object_java_lang_Object])(_r7.o, _r0.o, _r2.o);
    label29:;
    _r3.i = _r3.i + 1;
    goto label5;
    label32:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 229)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_getPackageName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_getPackageName__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "getPackageName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 232)
    // "org.apache.harmony.niochar.charset"
    _r0.o = xmlvm_create_java_string_from_pool(188);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_niochar_CharsetProviderImpl_getCharsetsInfo__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl_getCharsetsInfo__]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "getCharsetsInfo", "?")
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
    _r11.o = me;
    _r10.o = JAVA_NULL;
    _r9.i = 3;
    _r8.i = 2;
    _r7.i = 1;
    _r6.i = 0;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 252)
    _r1.i = 93;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object_1ARRAY, _r1.i);
    XMLVM_CLASS_INIT(java_lang_Object)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "US_ASCII"
    _r2.o = xmlvm_create_java_string_from_pool(189);
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r2.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r2.i = 13;
    XMLVM_CLASS_INIT(java_lang_String)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r2.i);
    // "US-ASCII"
    _r3.o = xmlvm_create_java_string_from_pool(190);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    // "ANSI_X3.4-1968"
    _r3.o = xmlvm_create_java_string_from_pool(191);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r3.o;
    // "ANSI_X3.4-1986"
    _r3.o = xmlvm_create_java_string_from_pool(192);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    // "iso-ir-6"
    _r3.o = xmlvm_create_java_string_from_pool(193);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r3.o;
    _r3.i = 4;
    // "iso_646.irv:1983"
    _r4.o = xmlvm_create_java_string_from_pool(194);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 5;
    // "ISO_646.irv:1991"
    _r4.o = xmlvm_create_java_string_from_pool(195);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 6;
    // "ASCII"
    _r4.o = xmlvm_create_java_string_from_pool(196);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 7;
    // "ISO646-US"
    _r4.o = xmlvm_create_java_string_from_pool(197);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 8;
    // "us"
    _r4.o = xmlvm_create_java_string_from_pool(198);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 9;
    // "cp367"
    _r4.o = xmlvm_create_java_string_from_pool(199);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 10;
    // "ascii7"
    _r4.o = xmlvm_create_java_string_from_pool(200);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 11;
    // "646"
    _r4.o = xmlvm_create_java_string_from_pool(201);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    _r3.i = 12;
    // "csASCII"
    _r4.o = xmlvm_create_java_string_from_pool(202);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r4.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r2.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r6.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r1.o;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "KOI8_R"
    _r2.o = xmlvm_create_java_string_from_pool(203);
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r2.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "KOI8-R"
    _r3.o = xmlvm_create_java_string_from_pool(204);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    // "csKOI8R"
    _r3.o = xmlvm_create_java_string_from_pool(205);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r3.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r2.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r7.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r1.o;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1250"
    _r2.o = xmlvm_create_java_string_from_pool(206);
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r2.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1250"
    _r3.o = xmlvm_create_java_string_from_pool(207);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    // "cp1250"
    _r3.o = xmlvm_create_java_string_from_pool(208);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r3.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r2.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r8.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r1.o;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r1.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1251"
    _r2.o = xmlvm_create_java_string_from_pool(209);
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r2.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1251"
    _r3.o = xmlvm_create_java_string_from_pool(210);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    // "cp1251"
    _r3.o = xmlvm_create_java_string_from_pool(211);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r3.o;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r2.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r9.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r1.o;
    _r1.i = 4;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1252"
    _r3.o = xmlvm_create_java_string_from_pool(212);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1252"
    _r4.o = xmlvm_create_java_string_from_pool(213);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1252"
    _r4.o = xmlvm_create_java_string_from_pool(214);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 5;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1253"
    _r3.o = xmlvm_create_java_string_from_pool(215);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1253"
    _r4.o = xmlvm_create_java_string_from_pool(216);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1253"
    _r4.o = xmlvm_create_java_string_from_pool(217);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 6;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1254"
    _r3.o = xmlvm_create_java_string_from_pool(218);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1254"
    _r4.o = xmlvm_create_java_string_from_pool(219);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1254"
    _r4.o = xmlvm_create_java_string_from_pool(220);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 7;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "CP_1257"
    _r3.o = xmlvm_create_java_string_from_pool(221);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1257"
    _r4.o = xmlvm_create_java_string_from_pool(222);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1257"
    _r4.o = xmlvm_create_java_string_from_pool(223);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 8;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_1"
    _r3.o = xmlvm_create_java_string_from_pool(224);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 15;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-1"
    _r4.o = xmlvm_create_java_string_from_pool(175);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "8859_1"
    _r4.o = xmlvm_create_java_string_from_pool(225);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO8859-1"
    _r4.o = xmlvm_create_java_string_from_pool(111);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ISO8859_1"
    _r4.o = xmlvm_create_java_string_from_pool(116);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-1:1987"
    _r5.o = xmlvm_create_java_string_from_pool(226);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "iso-ir-100"
    _r5.o = xmlvm_create_java_string_from_pool(227);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "ISO_8859-1"
    _r5.o = xmlvm_create_java_string_from_pool(228);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "latin1"
    _r5.o = xmlvm_create_java_string_from_pool(229);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 8;
    // "l1"
    _r5.o = xmlvm_create_java_string_from_pool(230);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 9;
    // "IBM819"
    _r5.o = xmlvm_create_java_string_from_pool(231);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 10;
    // "ISO_8859_1"
    _r5.o = xmlvm_create_java_string_from_pool(224);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 11;
    // "IBM-819"
    _r5.o = xmlvm_create_java_string_from_pool(232);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 12;
    // "CP819"
    _r5.o = xmlvm_create_java_string_from_pool(233);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 13;
    // "819"
    _r5.o = xmlvm_create_java_string_from_pool(234);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 14;
    // "csISOLatin1"
    _r5.o = xmlvm_create_java_string_from_pool(235);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 9;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_2"
    _r3.o = xmlvm_create_java_string_from_pool(236);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 8;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-2"
    _r4.o = xmlvm_create_java_string_from_pool(237);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "8859_2"
    _r4.o = xmlvm_create_java_string_from_pool(238);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-2:1987"
    _r4.o = xmlvm_create_java_string_from_pool(239);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-101"
    _r4.o = xmlvm_create_java_string_from_pool(240);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-2"
    _r5.o = xmlvm_create_java_string_from_pool(241);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "latin2"
    _r5.o = xmlvm_create_java_string_from_pool(242);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "l2"
    _r5.o = xmlvm_create_java_string_from_pool(243);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "csISOLatin2"
    _r5.o = xmlvm_create_java_string_from_pool(244);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 10;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_4"
    _r3.o = xmlvm_create_java_string_from_pool(245);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 8;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-4"
    _r4.o = xmlvm_create_java_string_from_pool(246);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "8859_4"
    _r4.o = xmlvm_create_java_string_from_pool(247);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-4:1988"
    _r4.o = xmlvm_create_java_string_from_pool(248);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-110"
    _r4.o = xmlvm_create_java_string_from_pool(249);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-4"
    _r5.o = xmlvm_create_java_string_from_pool(250);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "latin4"
    _r5.o = xmlvm_create_java_string_from_pool(251);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "l4"
    _r5.o = xmlvm_create_java_string_from_pool(252);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "csISOLatin4"
    _r5.o = xmlvm_create_java_string_from_pool(253);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 11;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_5"
    _r3.o = xmlvm_create_java_string_from_pool(254);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 7;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-5"
    _r4.o = xmlvm_create_java_string_from_pool(255);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "8859_5"
    _r4.o = xmlvm_create_java_string_from_pool(256);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-5:1988"
    _r4.o = xmlvm_create_java_string_from_pool(257);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-144"
    _r4.o = xmlvm_create_java_string_from_pool(258);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-5"
    _r5.o = xmlvm_create_java_string_from_pool(259);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "cyrillic"
    _r5.o = xmlvm_create_java_string_from_pool(260);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "csISOLatinCyrillic"
    _r5.o = xmlvm_create_java_string_from_pool(261);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 12;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_7"
    _r3.o = xmlvm_create_java_string_from_pool(262);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 9;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-7"
    _r4.o = xmlvm_create_java_string_from_pool(263);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ISO_8859-7:1987"
    _r4.o = xmlvm_create_java_string_from_pool(264);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-7"
    _r4.o = xmlvm_create_java_string_from_pool(265);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-126"
    _r4.o = xmlvm_create_java_string_from_pool(266);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ELOT_928"
    _r5.o = xmlvm_create_java_string_from_pool(267);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "ECMA-118"
    _r5.o = xmlvm_create_java_string_from_pool(268);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "greek"
    _r5.o = xmlvm_create_java_string_from_pool(269);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "greek8"
    _r5.o = xmlvm_create_java_string_from_pool(270);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 8;
    // "csISOLatinGreek"
    _r5.o = xmlvm_create_java_string_from_pool(271);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 13;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_9"
    _r3.o = xmlvm_create_java_string_from_pool(272);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 7;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-9"
    _r4.o = xmlvm_create_java_string_from_pool(273);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ISO_8859-9:1989"
    _r4.o = xmlvm_create_java_string_from_pool(274);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "iso-ir-148"
    _r4.o = xmlvm_create_java_string_from_pool(275);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ISO_8859-9"
    _r4.o = xmlvm_create_java_string_from_pool(276);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "latin5"
    _r5.o = xmlvm_create_java_string_from_pool(277);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "l5"
    _r5.o = xmlvm_create_java_string_from_pool(278);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "csISOLatin5"
    _r5.o = xmlvm_create_java_string_from_pool(279);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 14;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_13"
    _r3.o = xmlvm_create_java_string_from_pool(280);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r7.i);
    // "ISO-8859-13"
    _r4.o = xmlvm_create_java_string_from_pool(281);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 15;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "ISO_8859_15"
    _r3.o = xmlvm_create_java_string_from_pool(282);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "ISO-8859-15"
    _r4.o = xmlvm_create_java_string_from_pool(283);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ISO_8859-15"
    _r4.o = xmlvm_create_java_string_from_pool(284);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "Latin-9"
    _r4.o = xmlvm_create_java_string_from_pool(285);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 16;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "UTF_8"
    _r3.o = xmlvm_create_java_string_from_pool(286);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "UTF-8"
    _r4.o = xmlvm_create_java_string_from_pool(100);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "UTF8"
    _r4.o = xmlvm_create_java_string_from_pool(287);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 17;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "UTF_16"
    _r3.o = xmlvm_create_java_string_from_pool(288);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "UTF-16"
    _r4.o = xmlvm_create_java_string_from_pool(289);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "UTF16"
    _r4.o = xmlvm_create_java_string_from_pool(290);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "UTF_16"
    _r4.o = xmlvm_create_java_string_from_pool(288);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 18;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "UTF_16LE"
    _r3.o = xmlvm_create_java_string_from_pool(291);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "UTF-16LE"
    _r4.o = xmlvm_create_java_string_from_pool(292);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "X-UTF-16LE"
    _r4.o = xmlvm_create_java_string_from_pool(293);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "UTF_16LE"
    _r4.o = xmlvm_create_java_string_from_pool(291);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 19;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "UTF_16BE"
    _r3.o = xmlvm_create_java_string_from_pool(294);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "UTF-16BE"
    _r4.o = xmlvm_create_java_string_from_pool(295);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "X-UTF-16BE"
    _r4.o = xmlvm_create_java_string_from_pool(296);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "UTF_16BE"
    _r4.o = xmlvm_create_java_string_from_pool(294);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 20;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "IBM866"
    _r3.o = xmlvm_create_java_string_from_pool(297);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM866"
    _r4.o = xmlvm_create_java_string_from_pool(297);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp866"
    _r4.o = xmlvm_create_java_string_from_pool(298);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "866"
    _r4.o = xmlvm_create_java_string_from_pool(299);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM866"
    _r4.o = xmlvm_create_java_string_from_pool(300);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 21;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.windows_1255"
    _r3.o = xmlvm_create_java_string_from_pool(301);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1255"
    _r4.o = xmlvm_create_java_string_from_pool(302);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1255"
    _r4.o = xmlvm_create_java_string_from_pool(303);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 22;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.windows_1256"
    _r3.o = xmlvm_create_java_string_from_pool(304);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "windows-1256"
    _r4.o = xmlvm_create_java_string_from_pool(305);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1256"
    _r4.o = xmlvm_create_java_string_from_pool(306);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 23;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM1026"
    _r3.o = xmlvm_create_java_string_from_pool(307);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "IBM1026"
    _r4.o = xmlvm_create_java_string_from_pool(308);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "CP1026"
    _r4.o = xmlvm_create_java_string_from_pool(309);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "csIBM1026"
    _r4.o = xmlvm_create_java_string_from_pool(310);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 24;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM1047"
    _r3.o = xmlvm_create_java_string_from_pool(311);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM1047"
    _r4.o = xmlvm_create_java_string_from_pool(312);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "1047"
    _r4.o = xmlvm_create_java_string_from_pool(313);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1047"
    _r4.o = xmlvm_create_java_string_from_pool(314);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ibm-1047"
    _r4.o = xmlvm_create_java_string_from_pool(315);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 25;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM037"
    _r3.o = xmlvm_create_java_string_from_pool(316);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 7;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM037"
    _r4.o = xmlvm_create_java_string_from_pool(317);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp037"
    _r4.o = xmlvm_create_java_string_from_pool(318);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-us"
    _r4.o = xmlvm_create_java_string_from_pool(319);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ebcdic-cp-ca"
    _r4.o = xmlvm_create_java_string_from_pool(320);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ebcdic-cp-wt"
    _r5.o = xmlvm_create_java_string_from_pool(321);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "ebcdic-cp-nl"
    _r5.o = xmlvm_create_java_string_from_pool(322);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "csIBM037"
    _r5.o = xmlvm_create_java_string_from_pool(323);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 26;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM424"
    _r3.o = xmlvm_create_java_string_from_pool(324);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM424"
    _r4.o = xmlvm_create_java_string_from_pool(325);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp424"
    _r4.o = xmlvm_create_java_string_from_pool(326);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-he"
    _r4.o = xmlvm_create_java_string_from_pool(327);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM424"
    _r4.o = xmlvm_create_java_string_from_pool(328);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 27;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM437"
    _r3.o = xmlvm_create_java_string_from_pool(329);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM437"
    _r4.o = xmlvm_create_java_string_from_pool(330);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp437"
    _r4.o = xmlvm_create_java_string_from_pool(331);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "437"
    _r4.o = xmlvm_create_java_string_from_pool(332);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csPC8CodePage437"
    _r4.o = xmlvm_create_java_string_from_pool(333);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 28;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM500"
    _r3.o = xmlvm_create_java_string_from_pool(334);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM500"
    _r4.o = xmlvm_create_java_string_from_pool(335);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "CP500"
    _r4.o = xmlvm_create_java_string_from_pool(336);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-be"
    _r4.o = xmlvm_create_java_string_from_pool(337);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ebcdic-cp-ch"
    _r4.o = xmlvm_create_java_string_from_pool(338);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM500"
    _r5.o = xmlvm_create_java_string_from_pool(339);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 29;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM775"
    _r3.o = xmlvm_create_java_string_from_pool(340);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "IBM775"
    _r4.o = xmlvm_create_java_string_from_pool(341);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp775"
    _r4.o = xmlvm_create_java_string_from_pool(342);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "csPC775Baltic"
    _r4.o = xmlvm_create_java_string_from_pool(343);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 30;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM850"
    _r3.o = xmlvm_create_java_string_from_pool(344);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM850"
    _r4.o = xmlvm_create_java_string_from_pool(345);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp850"
    _r4.o = xmlvm_create_java_string_from_pool(346);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "850"
    _r4.o = xmlvm_create_java_string_from_pool(347);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csPC850Multilingual"
    _r4.o = xmlvm_create_java_string_from_pool(348);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 31;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM852"
    _r3.o = xmlvm_create_java_string_from_pool(349);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM852"
    _r4.o = xmlvm_create_java_string_from_pool(350);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp852"
    _r4.o = xmlvm_create_java_string_from_pool(351);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "852"
    _r4.o = xmlvm_create_java_string_from_pool(352);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csPCp852"
    _r4.o = xmlvm_create_java_string_from_pool(353);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 32;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM855"
    _r3.o = xmlvm_create_java_string_from_pool(354);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM855"
    _r4.o = xmlvm_create_java_string_from_pool(355);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp855"
    _r4.o = xmlvm_create_java_string_from_pool(356);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "855"
    _r4.o = xmlvm_create_java_string_from_pool(357);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM855"
    _r4.o = xmlvm_create_java_string_from_pool(358);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 33;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM857"
    _r3.o = xmlvm_create_java_string_from_pool(359);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM857"
    _r4.o = xmlvm_create_java_string_from_pool(360);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp857"
    _r4.o = xmlvm_create_java_string_from_pool(361);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "857"
    _r4.o = xmlvm_create_java_string_from_pool(362);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM857"
    _r4.o = xmlvm_create_java_string_from_pool(363);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 34;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM860"
    _r3.o = xmlvm_create_java_string_from_pool(364);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM860"
    _r4.o = xmlvm_create_java_string_from_pool(365);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp860"
    _r4.o = xmlvm_create_java_string_from_pool(366);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "860"
    _r4.o = xmlvm_create_java_string_from_pool(367);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM860"
    _r4.o = xmlvm_create_java_string_from_pool(368);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 35;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM861"
    _r3.o = xmlvm_create_java_string_from_pool(369);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM861"
    _r4.o = xmlvm_create_java_string_from_pool(370);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp861"
    _r4.o = xmlvm_create_java_string_from_pool(371);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "861"
    _r4.o = xmlvm_create_java_string_from_pool(372);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "cp-is"
    _r4.o = xmlvm_create_java_string_from_pool(373);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM861"
    _r5.o = xmlvm_create_java_string_from_pool(374);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 36;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM862"
    _r3.o = xmlvm_create_java_string_from_pool(375);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM862"
    _r4.o = xmlvm_create_java_string_from_pool(376);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp862"
    _r4.o = xmlvm_create_java_string_from_pool(377);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "862"
    _r4.o = xmlvm_create_java_string_from_pool(378);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csPC862LatinHebrew"
    _r4.o = xmlvm_create_java_string_from_pool(379);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 37;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM863"
    _r3.o = xmlvm_create_java_string_from_pool(380);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM863"
    _r4.o = xmlvm_create_java_string_from_pool(381);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp863"
    _r4.o = xmlvm_create_java_string_from_pool(382);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "863"
    _r4.o = xmlvm_create_java_string_from_pool(383);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM863"
    _r4.o = xmlvm_create_java_string_from_pool(384);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 38;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM865"
    _r3.o = xmlvm_create_java_string_from_pool(385);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM865"
    _r4.o = xmlvm_create_java_string_from_pool(386);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp865"
    _r4.o = xmlvm_create_java_string_from_pool(387);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "865"
    _r4.o = xmlvm_create_java_string_from_pool(388);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM865"
    _r4.o = xmlvm_create_java_string_from_pool(389);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 39;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM869"
    _r3.o = xmlvm_create_java_string_from_pool(390);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM869"
    _r4.o = xmlvm_create_java_string_from_pool(391);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp869"
    _r4.o = xmlvm_create_java_string_from_pool(392);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "869"
    _r4.o = xmlvm_create_java_string_from_pool(393);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "cp-gr"
    _r4.o = xmlvm_create_java_string_from_pool(394);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM869"
    _r5.o = xmlvm_create_java_string_from_pool(395);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 40;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM00858"
    _r3.o = xmlvm_create_java_string_from_pool(396);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM00858"
    _r4.o = xmlvm_create_java_string_from_pool(397);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp858"
    _r4.o = xmlvm_create_java_string_from_pool(398);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID00858"
    _r4.o = xmlvm_create_java_string_from_pool(399);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP00858"
    _r4.o = xmlvm_create_java_string_from_pool(400);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 41;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01140"
    _r3.o = xmlvm_create_java_string_from_pool(401);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01140"
    _r4.o = xmlvm_create_java_string_from_pool(402);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1140"
    _r4.o = xmlvm_create_java_string_from_pool(403);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01140"
    _r4.o = xmlvm_create_java_string_from_pool(404);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01140"
    _r4.o = xmlvm_create_java_string_from_pool(405);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 42;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01141"
    _r3.o = xmlvm_create_java_string_from_pool(406);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01141"
    _r4.o = xmlvm_create_java_string_from_pool(407);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1141"
    _r4.o = xmlvm_create_java_string_from_pool(408);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01141"
    _r4.o = xmlvm_create_java_string_from_pool(409);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01141"
    _r4.o = xmlvm_create_java_string_from_pool(410);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 43;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01142"
    _r3.o = xmlvm_create_java_string_from_pool(411);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01142"
    _r4.o = xmlvm_create_java_string_from_pool(412);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1142"
    _r4.o = xmlvm_create_java_string_from_pool(413);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01142"
    _r4.o = xmlvm_create_java_string_from_pool(414);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01142"
    _r4.o = xmlvm_create_java_string_from_pool(415);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 44;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01143"
    _r3.o = xmlvm_create_java_string_from_pool(416);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01143"
    _r4.o = xmlvm_create_java_string_from_pool(417);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1143"
    _r4.o = xmlvm_create_java_string_from_pool(418);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01143"
    _r4.o = xmlvm_create_java_string_from_pool(419);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01143"
    _r4.o = xmlvm_create_java_string_from_pool(420);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 45;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01144"
    _r3.o = xmlvm_create_java_string_from_pool(421);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01144"
    _r4.o = xmlvm_create_java_string_from_pool(422);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1144"
    _r4.o = xmlvm_create_java_string_from_pool(423);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01144"
    _r4.o = xmlvm_create_java_string_from_pool(424);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01144"
    _r4.o = xmlvm_create_java_string_from_pool(425);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 46;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01145"
    _r3.o = xmlvm_create_java_string_from_pool(426);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01145"
    _r4.o = xmlvm_create_java_string_from_pool(427);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1145"
    _r4.o = xmlvm_create_java_string_from_pool(428);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01145"
    _r4.o = xmlvm_create_java_string_from_pool(429);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01145"
    _r4.o = xmlvm_create_java_string_from_pool(430);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 47;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01146"
    _r3.o = xmlvm_create_java_string_from_pool(431);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01146"
    _r4.o = xmlvm_create_java_string_from_pool(432);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1146"
    _r4.o = xmlvm_create_java_string_from_pool(433);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01146"
    _r4.o = xmlvm_create_java_string_from_pool(434);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01146"
    _r4.o = xmlvm_create_java_string_from_pool(435);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 48;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01147"
    _r3.o = xmlvm_create_java_string_from_pool(436);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01147"
    _r4.o = xmlvm_create_java_string_from_pool(437);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1147"
    _r4.o = xmlvm_create_java_string_from_pool(438);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01147"
    _r4.o = xmlvm_create_java_string_from_pool(439);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01147"
    _r4.o = xmlvm_create_java_string_from_pool(440);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 49;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01148"
    _r3.o = xmlvm_create_java_string_from_pool(441);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01148"
    _r4.o = xmlvm_create_java_string_from_pool(442);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1148"
    _r4.o = xmlvm_create_java_string_from_pool(443);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01148"
    _r4.o = xmlvm_create_java_string_from_pool(444);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01148"
    _r4.o = xmlvm_create_java_string_from_pool(445);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 50;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM01149"
    _r3.o = xmlvm_create_java_string_from_pool(446);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM01149"
    _r4.o = xmlvm_create_java_string_from_pool(447);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1149"
    _r4.o = xmlvm_create_java_string_from_pool(448);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "CCSID01149"
    _r4.o = xmlvm_create_java_string_from_pool(449);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "CP01149"
    _r4.o = xmlvm_create_java_string_from_pool(450);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 51;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM273"
    _r3.o = xmlvm_create_java_string_from_pool(451);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "IBM273"
    _r4.o = xmlvm_create_java_string_from_pool(452);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp273"
    _r4.o = xmlvm_create_java_string_from_pool(453);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "csIBM273"
    _r4.o = xmlvm_create_java_string_from_pool(454);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 52;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM277"
    _r3.o = xmlvm_create_java_string_from_pool(455);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM277"
    _r4.o = xmlvm_create_java_string_from_pool(456);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp277"
    _r4.o = xmlvm_create_java_string_from_pool(457);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "EBCDIC-CP-DK"
    _r4.o = xmlvm_create_java_string_from_pool(458);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "EBCDIC-CP-NO"
    _r4.o = xmlvm_create_java_string_from_pool(459);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM277"
    _r5.o = xmlvm_create_java_string_from_pool(460);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 53;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM278"
    _r3.o = xmlvm_create_java_string_from_pool(461);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM278"
    _r4.o = xmlvm_create_java_string_from_pool(462);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp278"
    _r4.o = xmlvm_create_java_string_from_pool(463);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-fi"
    _r4.o = xmlvm_create_java_string_from_pool(464);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ebcdic-cp-se"
    _r4.o = xmlvm_create_java_string_from_pool(465);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM278"
    _r5.o = xmlvm_create_java_string_from_pool(466);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 54;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM280"
    _r3.o = xmlvm_create_java_string_from_pool(467);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM280"
    _r4.o = xmlvm_create_java_string_from_pool(468);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp280"
    _r4.o = xmlvm_create_java_string_from_pool(469);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-it"
    _r4.o = xmlvm_create_java_string_from_pool(470);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM280"
    _r4.o = xmlvm_create_java_string_from_pool(471);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 55;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM284"
    _r3.o = xmlvm_create_java_string_from_pool(472);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM284"
    _r4.o = xmlvm_create_java_string_from_pool(473);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp284"
    _r4.o = xmlvm_create_java_string_from_pool(474);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-es"
    _r4.o = xmlvm_create_java_string_from_pool(475);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM284"
    _r4.o = xmlvm_create_java_string_from_pool(476);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 56;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM285"
    _r3.o = xmlvm_create_java_string_from_pool(477);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM285"
    _r4.o = xmlvm_create_java_string_from_pool(478);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp285"
    _r4.o = xmlvm_create_java_string_from_pool(479);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-gb"
    _r4.o = xmlvm_create_java_string_from_pool(480);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM285"
    _r4.o = xmlvm_create_java_string_from_pool(481);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 57;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM297"
    _r3.o = xmlvm_create_java_string_from_pool(482);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM297"
    _r4.o = xmlvm_create_java_string_from_pool(483);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp297"
    _r4.o = xmlvm_create_java_string_from_pool(484);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-fr"
    _r4.o = xmlvm_create_java_string_from_pool(485);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM297"
    _r4.o = xmlvm_create_java_string_from_pool(486);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 58;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM870"
    _r3.o = xmlvm_create_java_string_from_pool(487);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 5;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM870"
    _r4.o = xmlvm_create_java_string_from_pool(488);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp870"
    _r4.o = xmlvm_create_java_string_from_pool(489);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-roece"
    _r4.o = xmlvm_create_java_string_from_pool(490);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ebcdic-cp-yu"
    _r4.o = xmlvm_create_java_string_from_pool(491);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "csIBM870"
    _r5.o = xmlvm_create_java_string_from_pool(492);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 59;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM871"
    _r3.o = xmlvm_create_java_string_from_pool(493);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM871"
    _r4.o = xmlvm_create_java_string_from_pool(494);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp871"
    _r4.o = xmlvm_create_java_string_from_pool(495);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-is"
    _r4.o = xmlvm_create_java_string_from_pool(496);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM871"
    _r4.o = xmlvm_create_java_string_from_pool(497);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 60;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM918"
    _r3.o = xmlvm_create_java_string_from_pool(498);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM918"
    _r4.o = xmlvm_create_java_string_from_pool(499);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp918"
    _r4.o = xmlvm_create_java_string_from_pool(500);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-ar2"
    _r4.o = xmlvm_create_java_string_from_pool(501);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM918"
    _r4.o = xmlvm_create_java_string_from_pool(502);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 61;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM420"
    _r3.o = xmlvm_create_java_string_from_pool(503);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM420"
    _r4.o = xmlvm_create_java_string_from_pool(504);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp420"
    _r4.o = xmlvm_create_java_string_from_pool(505);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ebcdic-cp-ar1"
    _r4.o = xmlvm_create_java_string_from_pool(506);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM420"
    _r4.o = xmlvm_create_java_string_from_pool(507);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 62;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM864"
    _r3.o = xmlvm_create_java_string_from_pool(508);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "IBM864"
    _r4.o = xmlvm_create_java_string_from_pool(509);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp864"
    _r4.o = xmlvm_create_java_string_from_pool(510);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "csIBM864"
    _r4.o = xmlvm_create_java_string_from_pool(511);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 63;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM868"
    _r3.o = xmlvm_create_java_string_from_pool(512);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 4;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "IBM868"
    _r4.o = xmlvm_create_java_string_from_pool(513);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp868"
    _r4.o = xmlvm_create_java_string_from_pool(514);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp-ar"
    _r4.o = xmlvm_create_java_string_from_pool(515);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "csIBM868"
    _r4.o = xmlvm_create_java_string_from_pool(516);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 64;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.ISO_8859_3"
    _r3.o = xmlvm_create_java_string_from_pool(517);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 9;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-3"
    _r4.o = xmlvm_create_java_string_from_pool(518);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "8859_3"
    _r4.o = xmlvm_create_java_string_from_pool(519);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859_3"
    _r4.o = xmlvm_create_java_string_from_pool(520);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "ISO_8859_2:1998"
    _r4.o = xmlvm_create_java_string_from_pool(521);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "iso-ir-109"
    _r5.o = xmlvm_create_java_string_from_pool(522);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "ISO_8859-3"
    _r5.o = xmlvm_create_java_string_from_pool(523);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "latin3"
    _r5.o = xmlvm_create_java_string_from_pool(524);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "l3"
    _r5.o = xmlvm_create_java_string_from_pool(525);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 8;
    // "csISOLatin3"
    _r5.o = xmlvm_create_java_string_from_pool(526);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 65;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.ISO_8859_6"
    _r3.o = xmlvm_create_java_string_from_pool(527);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 9;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-6"
    _r4.o = xmlvm_create_java_string_from_pool(528);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ISO_8859_6"
    _r4.o = xmlvm_create_java_string_from_pool(529);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-6:1987"
    _r4.o = xmlvm_create_java_string_from_pool(530);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-127"
    _r4.o = xmlvm_create_java_string_from_pool(531);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-6"
    _r5.o = xmlvm_create_java_string_from_pool(532);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "ECMA-114"
    _r5.o = xmlvm_create_java_string_from_pool(533);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "ASMO-708"
    _r5.o = xmlvm_create_java_string_from_pool(534);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "arabic"
    _r5.o = xmlvm_create_java_string_from_pool(535);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 8;
    // "csISOLatinArabic"
    _r5.o = xmlvm_create_java_string_from_pool(536);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 66;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.ISO_8859_8"
    _r3.o = xmlvm_create_java_string_from_pool(537);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 7;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "ISO-8859-8"
    _r4.o = xmlvm_create_java_string_from_pool(538);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ISO_8859_8"
    _r4.o = xmlvm_create_java_string_from_pool(539);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ISO_8859-8:1988"
    _r4.o = xmlvm_create_java_string_from_pool(540);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "iso-ir-138"
    _r4.o = xmlvm_create_java_string_from_pool(541);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "ISO_8859-8"
    _r5.o = xmlvm_create_java_string_from_pool(542);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "hebrew"
    _r5.o = xmlvm_create_java_string_from_pool(543);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "csISOLatinHebrew"
    _r5.o = xmlvm_create_java_string_from_pool(544);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 67;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.IBM_Thai"
    _r3.o = xmlvm_create_java_string_from_pool(545);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "IBM-Thai"
    _r4.o = xmlvm_create_java_string_from_pool(546);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp838"
    _r4.o = xmlvm_create_java_string_from_pool(547);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 68;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM737"
    _r3.o = xmlvm_create_java_string_from_pool(548);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM737"
    _r4.o = xmlvm_create_java_string_from_pool(549);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-737_P100-1997"
    _r4.o = xmlvm_create_java_string_from_pool(550);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp737"
    _r4.o = xmlvm_create_java_string_from_pool(551);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 69;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM856"
    _r3.o = xmlvm_create_java_string_from_pool(552);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "x-IBM856"
    _r4.o = xmlvm_create_java_string_from_pool(553);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp856"
    _r4.o = xmlvm_create_java_string_from_pool(554);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 70;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM874"
    _r3.o = xmlvm_create_java_string_from_pool(555);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "TIS-620"
    _r4.o = xmlvm_create_java_string_from_pool(556);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-IBM874"
    _r4.o = xmlvm_create_java_string_from_pool(557);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp874"
    _r4.o = xmlvm_create_java_string_from_pool(558);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 71;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM875"
    _r3.o = xmlvm_create_java_string_from_pool(559);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM875"
    _r4.o = xmlvm_create_java_string_from_pool(560);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-875_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(561);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp875"
    _r4.o = xmlvm_create_java_string_from_pool(562);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 72;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM922"
    _r3.o = xmlvm_create_java_string_from_pool(563);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "x-IBM922"
    _r4.o = xmlvm_create_java_string_from_pool(564);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp922"
    _r4.o = xmlvm_create_java_string_from_pool(565);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 73;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1006"
    _r3.o = xmlvm_create_java_string_from_pool(566);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1006"
    _r4.o = xmlvm_create_java_string_from_pool(567);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1006_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(568);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1006"
    _r4.o = xmlvm_create_java_string_from_pool(569);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 74;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1025"
    _r3.o = xmlvm_create_java_string_from_pool(570);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1025"
    _r4.o = xmlvm_create_java_string_from_pool(571);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1025_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(572);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1025"
    _r4.o = xmlvm_create_java_string_from_pool(573);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 75;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1112"
    _r3.o = xmlvm_create_java_string_from_pool(574);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1112"
    _r4.o = xmlvm_create_java_string_from_pool(575);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1112_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(576);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1112"
    _r4.o = xmlvm_create_java_string_from_pool(577);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 76;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1122"
    _r3.o = xmlvm_create_java_string_from_pool(578);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1122"
    _r4.o = xmlvm_create_java_string_from_pool(579);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1122_P100-1999"
    _r4.o = xmlvm_create_java_string_from_pool(580);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1122"
    _r4.o = xmlvm_create_java_string_from_pool(581);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 77;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1123"
    _r3.o = xmlvm_create_java_string_from_pool(582);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1123"
    _r4.o = xmlvm_create_java_string_from_pool(583);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1123_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(584);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1123"
    _r4.o = xmlvm_create_java_string_from_pool(585);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 78;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1124"
    _r3.o = xmlvm_create_java_string_from_pool(586);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1124"
    _r4.o = xmlvm_create_java_string_from_pool(587);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1124_P100-1996"
    _r4.o = xmlvm_create_java_string_from_pool(588);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1124"
    _r4.o = xmlvm_create_java_string_from_pool(589);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 79;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1097"
    _r3.o = xmlvm_create_java_string_from_pool(590);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1097"
    _r4.o = xmlvm_create_java_string_from_pool(591);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1097_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(592);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1097"
    _r4.o = xmlvm_create_java_string_from_pool(593);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 80;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_IBM1098"
    _r3.o = xmlvm_create_java_string_from_pool(594);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-IBM1098"
    _r4.o = xmlvm_create_java_string_from_pool(595);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-ibm-1098_P100-1995"
    _r4.o = xmlvm_create_java_string_from_pool(596);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "cp1098"
    _r4.o = xmlvm_create_java_string_from_pool(597);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 81;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_MacCyrillic"
    _r3.o = xmlvm_create_java_string_from_pool(598);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-MacCyrillic"
    _r4.o = xmlvm_create_java_string_from_pool(599);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-mac-cyrillic"
    _r4.o = xmlvm_create_java_string_from_pool(600);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "MacCyrillic"
    _r4.o = xmlvm_create_java_string_from_pool(601);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 82;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_MacGreek"
    _r3.o = xmlvm_create_java_string_from_pool(602);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-MacGreek"
    _r4.o = xmlvm_create_java_string_from_pool(603);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-mac-greek"
    _r4.o = xmlvm_create_java_string_from_pool(604);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "MacGreek"
    _r4.o = xmlvm_create_java_string_from_pool(605);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 83;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_MacTurkish"
    _r3.o = xmlvm_create_java_string_from_pool(606);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-MacTurkish"
    _r4.o = xmlvm_create_java_string_from_pool(607);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-mac-turkish"
    _r4.o = xmlvm_create_java_string_from_pool(608);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "MacTurkish"
    _r4.o = xmlvm_create_java_string_from_pool(609);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 84;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.windows_31j"
    _r3.o = xmlvm_create_java_string_from_pool(610);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    _r3.i = 9;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r3.i);
    // "Shift_JIS"
    _r4.o = xmlvm_create_java_string_from_pool(611);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "windows-31j"
    _r4.o = xmlvm_create_java_string_from_pool(612);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "MS932"
    _r4.o = xmlvm_create_java_string_from_pool(613);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    // "windows-932"
    _r4.o = xmlvm_create_java_string_from_pool(614);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r9.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r9.i] = _r4.o;
    _r4.i = 4;
    // "cp932"
    _r5.o = xmlvm_create_java_string_from_pool(615);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 5;
    // "csWindows31J"
    _r5.o = xmlvm_create_java_string_from_pool(616);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 6;
    // "cp943c"
    _r5.o = xmlvm_create_java_string_from_pool(617);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 7;
    // "x-ms-cp932"
    _r5.o = xmlvm_create_java_string_from_pool(618);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    _r4.i = 8;
    // "ibm-943"
    _r5.o = xmlvm_create_java_string_from_pool(619);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r5.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 85;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.Big5"
    _r3.o = xmlvm_create_java_string_from_pool(620);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "Big5"
    _r4.o = xmlvm_create_java_string_from_pool(621);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "csBig5"
    _r4.o = xmlvm_create_java_string_from_pool(622);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "windows-950"
    _r4.o = xmlvm_create_java_string_from_pool(623);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 86;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.Big5_HKSCS"
    _r3.o = xmlvm_create_java_string_from_pool(624);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "Big5-HKSCS"
    _r4.o = xmlvm_create_java_string_from_pool(625);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "ibm-1375"
    _r4.o = xmlvm_create_java_string_from_pool(626);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 87;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.EUC_KR"
    _r3.o = xmlvm_create_java_string_from_pool(627);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "EUC-KR"
    _r4.o = xmlvm_create_java_string_from_pool(628);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "windows-51949"
    _r4.o = xmlvm_create_java_string_from_pool(629);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ibm-970"
    _r4.o = xmlvm_create_java_string_from_pool(630);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 88;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.GBK"
    _r3.o = xmlvm_create_java_string_from_pool(631);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r7.i);
    // "GBK"
    _r4.o = xmlvm_create_java_string_from_pool(632);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 89;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_MS950_HKSCS"
    _r3.o = xmlvm_create_java_string_from_pool(633);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "x-ibm-1375_P100-2003"
    _r4.o = xmlvm_create_java_string_from_pool(634);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "x-MS950-HKSCS"
    _r4.o = xmlvm_create_java_string_from_pool(635);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "MS950_HKSCS"
    _r4.o = xmlvm_create_java_string_from_pool(636);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 90;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.x_windows_949"
    _r3.o = xmlvm_create_java_string_from_pool(637);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r8.i);
    // "x-windows-949"
    _r4.o = xmlvm_create_java_string_from_pool(638);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "MS949"
    _r4.o = xmlvm_create_java_string_from_pool(639);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 91;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.GB18030"
    _r3.o = xmlvm_create_java_string_from_pool(640);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "GB18030"
    _r4.o = xmlvm_create_java_string_from_pool(641);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "windows-54936"
    _r4.o = xmlvm_create_java_string_from_pool(642);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "ibm-1392"
    _r4.o = xmlvm_create_java_string_from_pool(643);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 92;
    XMLVM_CLASS_INIT(java_lang_Object)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_Object, _r9.i);
    // "additional.GB2312"
    _r3.o = xmlvm_create_java_string_from_pool(644);
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r3.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r10.o;
    XMLVM_CLASS_INIT(java_lang_String)
    _r3.o = XMLVMArray_createSingleDimension(__CLASS_java_lang_String, _r9.i);
    // "GB2312"
    _r4.o = xmlvm_create_java_string_from_pool(645);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r6.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r4.o;
    // "cp1383"
    _r4.o = xmlvm_create_java_string_from_pool(646);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r7.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r4.o;
    // "EUC_CN"
    _r4.o = xmlvm_create_java_string_from_pool(647);
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r4.o;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r8.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r8.i] = _r3.o;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((org_xmlvm_runtime_XMLVMArray**) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 725)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_niochar_CharsetProviderImpl___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_niochar_CharsetProviderImpl___CLINIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.niochar.CharsetProviderImpl", "<clinit>", "?")
    volatile XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 50)
    _r0.i = 0;
    org_apache_harmony_niochar_CharsetProviderImpl_PUT_HAS_LOADED_NATIVES( _r0.i);
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 55)
    _r0.i = 1;
    XMLVM_TRY_BEGIN(w3635aaac16b1b6)
    // Begin try
    org_apache_harmony_niochar_CharsetProviderImpl_PUT_HAS_LOADED_NATIVES( _r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w3635aaac16b1b6)
    XMLVM_CATCH_END(w3635aaac16b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w3635aaac16b1b6)
    label6:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 59)
    XMLVM_EXIT_METHOD()
    return;
    label7:;
    XMLVM_SOURCE_POSITION("CharsetProviderImpl.java", 56)
    java_lang_Thread* curThread_w3635aaac16b1c12 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w3635aaac16b1c12->fields.java_lang_Thread.xmlvmException_;
    goto label6;
    //XMLVM_END_WRAPPER
}

